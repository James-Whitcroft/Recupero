/* this class does all the heavy lifting for recupero.
This class will read, parse and output everything that is called.
@author: samuel Launt
@author: James Whitcroft
*/

#include "ext2_fs.h"
#include "Extended2.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <bitset>
#define OFFSET 1024
using namespace std;

void Extended2::meta(const char* path){
	//attempt to open partition, read only
	//exit -1 on fail
	if((this->driveFileHandle=open(path, O_RDONLY))<0){
		cout << "Failed :: cannot open " << path << endl;
		exit(-1);
	}
	//seek to start of super block and read
	lseek(driveFileHandle, OFFSET, SEEK_SET);
	read(driveFileHandle, &this->superBlock, sizeof(ext2_super_block));
	//confirm magic number for valid ext2
	//exit -1 if not ext2
	if(this->superBlock.s_magic != EXT2_SUPER_MAGIC){
		cout << "Failed :: bad magic number " << this->superBlock.s_magic << endl;
		exit(-1);
	}
	//ext2 confirmed, get meta data
	//block size
	this->blockSize = OFFSET << this->superBlock.s_log_block_size;
	//number of block groups
	this->blockGroupCount = 1+(this->superBlock.s_blocks_count-1)/this->superBlock.s_blocks_per_group;
	//group descriptor size in bytes
	descriptorSize = blockGroupCount*sizeof(ext2_group_desc);
	if(blockSize==OFFSET){	
		lseek(driveFileHandle, OFFSET+this->blockSize, SEEK_SET);
	}else{
		lseek(driveFileHandle, this->blockSize, SEEK_SET);
	}
	for(unsigned int i=0; i<this->blockGroupCount; i++){
		read(driveFileHandle, &descriptorBlock, sizeof(ext2_group_desc));
		this->iNodeBitMapLocations.push_back(descriptorBlock.bg_inode_bitmap);
		this->iNodeTableLocations.push_back(descriptorBlock.bg_inode_table);
		this->dataBlockBitMapLocations.push_back(descriptorBlock.bg_block_bitmap);
	}
	cout << endl;
}
Extended2::Extended2(const char* path){
	this->meta(path);
}

Extended2::Extended2(const char* path, int byteOffset){	
	recoverSuper(path, byteOffset);
	meta(path);
}

Extended2::~Extended2(){
	close(driveFileHandle);
}

void Extended2::printInode(unsigned int num){
	unsigned int iNodeBlockCount;
  unsigned int iNodesPerGroup;
  unsigned int pause=1;
	time_t mtime;
	time_t atime;
	time_t ctime;
	struct tm *ptm;
  int div;
  if(blockSize==OFFSET){div=2;}else{div=7;}
  //num of inode per block
  iNodesPerGroup=this->blockSize/sizeof(ext2_inode);
  //cout<<iNodesPerGroup<<endl;
  //num of blocks in table
  iNodeBlockCount=this->superBlock.s_inodes_per_group/iNodesPerGroup;
  //cout<<iNodeBlockCount <<endl;
	for(unsigned int i=0; i<this->iNodeTableLocations.size(); i++){
    for(unsigned int x=0; x<iNodeBlockCount; x++){
      for(unsigned int y=0; y<iNodesPerGroup; y++){
        if(pause==num){
          if(OFFSET==blockSize){
            lseek(driveFileHandle, ((x+this->iNodeTableLocations.at(i))*this->blockSize)+(y*sizeof(ext2_inode)), SEEK_SET);
          }else{
            lseek(driveFileHandle, ((blockSize*x+this->iNodeTableLocations.at(i))*this->blockSize)+(y*sizeof(ext2_inode)), SEEK_SET);
          }
          read(driveFileHandle, &inodeStruct, sizeof(ext2_inode));
          mtime=inodeStruct.i_mtime;
          atime=inodeStruct.i_atime;
          ctime=inodeStruct.i_ctime;
          ptm=gmtime(&mtime);
          if(S_ISDIR(inodeStruct.i_mode)){cout << "d";}
          else if(S_ISREG(inodeStruct.i_mode)){cout << "r";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IRUSR){cout << "r";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IWUSR){cout << "w";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IXUSR){cout << "x";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IRGRP){cout << "r";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IWGRP){cout << "w";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IXGRP){cout << "x";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IROTH){cout << "r";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IWOTH){cout << "w";}else{cout << "-";}
          if(inodeStruct.i_mode & S_IXOTH){cout << "x";}else{cout << "-";}
          cout << endl;
          cout << "Size -> " << inodeStruct.i_size << endl;
          cout << "Modify time -> " << (ptm->tm_hour%12)+5 <<":"<< ptm->tm_min <<" "<< ptm->tm_year+1900 << endl;
          ptm=gmtime(&atime);
          cout << "Access time -> " << (ptm->tm_hour%12)+5 <<":"<< ptm->tm_min <<" "<< ptm->tm_year+1900 << endl;
          ptm=gmtime(&ctime);
          cout << "Creation time -> " << (ptm->tm_hour%12)+5 <<":"<< ptm->tm_min <<" "<< ptm->tm_year+1900 << endl;
          cout << "Link count -> " << inodeStruct.i_links_count << endl;
          cout << "Block count -> " << inodeStruct.i_blocks/div << endl;
          /*cout << "Block count -> " << inodeStruct.i_blocks/div << endl;
          for(unsigned int f=0;f<inodeStruct.i_blocks/div;f++){*/
          for(unsigned int f=0;f<15;f++){
            if (f == 12){
              cout << "Indirect Inode " << f+1 << " -> " << inodeStruct.i_block[f] << endl;
            }//end if
            else if(f == 13){
              cout << "Double Indirect Inode " << f+1 << " -> " << inodeStruct.i_block[f] << endl;
            }
            else if(f == 14){
              cout << "Tripple Indirect Inode " << f+1 << " -> " << inodeStruct.i_block[f] << endl;
            }
            else{
              cout << "Block " << f+1 << " -> " << inodeStruct.i_block[f] << endl;
            }//end else
          }//end for
          return;
        }
        pause++;
      }
    }
  }
}

void Extended2::printGroup(){
	if(blockSize==OFFSET){	
		lseek(driveFileHandle, OFFSET+this->blockSize, SEEK_SET);
	}else{
		lseek(driveFileHandle, this->blockSize, SEEK_SET);
	}
	for(unsigned int i=0; i<this->blockGroupCount; i++){
		read(driveFileHandle, &descriptorBlock, sizeof(ext2_group_desc));
		cout << "GROUP DESCRIPTOR :: " << i << " :: " << endl;
		cout << "Free Block Count\t-> " << this->descriptorBlock.bg_free_blocks_count << endl;
		cout << "Free iNode Count\t-> " << this->descriptorBlock.bg_free_inodes_count << endl;
		cout << "iNode Bitmap Block\t-> " << this->descriptorBlock.bg_inode_bitmap << endl;
		cout << "Data Bitmap Block\t-> " << this->descriptorBlock.bg_block_bitmap << endl;
		cout << "iNode Table\t\t-> " << this->descriptorBlock.bg_inode_table << endl;
	}
	cout << endl;
}

void Extended2::printSuper(){
  printf(
   "Inodes count            : %u\n"
   "Blocks count            : %u\n"
   "Reserved blocks count   : %u\n"
   "Free blocks count       : %u\n"
   "Free inodes count       : %u\n"
   "First data block        : %u\n"
   "Block size              : %u\n"
   "Blocks per group        : %u\n"
   "Inodes per group        : %u\n"
   "Creator OS              : %u\n"
   "First non-reserved inode: %u\n"
   "Size of inode structure : %hu\n"
   ,
   this->superBlock.s_inodes_count,  
   this->superBlock.s_blocks_count,
   this->superBlock.s_r_blocks_count,     /* reserved blocks count */
   this->superBlock.s_free_blocks_count,
   this->superBlock.s_free_inodes_count,
   this->superBlock.s_first_data_block,
   this->blockSize,
   this->superBlock.s_blocks_per_group,
   this->superBlock.s_inodes_per_group,
   this->superBlock.s_creator_os,
   this->superBlock.s_first_ino,          /* first non-reserved inode */
   this->superBlock.s_inode_size);	
}


/*
*
*@params enable, 0=print all deleted to stdout; 1=recover all deleted, no print
*
*/
void Extended2::recoverData(int enable){
  unsigned int iNodeBlockCount;
  unsigned int iNodesPerGroup;
  unsigned int div;
  unsigned int optnum=1;
  unsigned int pause=1;
  unsigned char blockholder[blockSize];
	string c;
	ofstream outfile;
	iNodesPerGroup=this->blockSize/sizeof(ext2_inode);
  iNodeBlockCount=this->superBlock.s_inodes_per_group/iNodesPerGroup;
  if(enable){mkdir("Recupero", S_IRWXU | S_IRWXG | S_IRWXO);}
  for(unsigned int i=0; i<this->iNodeTableLocations.size(); i++){
    for(unsigned int x=0; x<iNodeBlockCount; x++){
      for(unsigned int y=0; y<iNodesPerGroup; y++){
        lseek(driveFileHandle, ((x+this->iNodeTableLocations.at(i))*this->blockSize)+(y*sizeof(ext2_inode)), SEEK_SET);
        read(driveFileHandle, &inodeStruct, sizeof(ext2_inode));
        if(!this->inodeStruct.i_dtime==0){
          if(!enable){
            if(blockSize==OFFSET){div=2;}else{div=7;}
            cout << optnum << "  |\tinode #: "<< pause << endl;
            for(unsigned int f=0;f<12;f++){
              int check = isFree(inodeStruct.i_block[f]);
              if(check == 0){
                cout << "\t\tBlock " << f << " is possibly corrupt!!"<< endl;
              }//end if
              else if(check == -1){
                cout << "\t\tSomething is really wrong with this one..." << endl;
                cout << "\t\tPointing to blocks that dont exist!" << endl;
                break;
              }
            }//end for
            optnum++;
          }else{
            stringstream s;
            s << pause;
            c="Recupero/"+s.str();
            outfile.open(c.c_str());
            if(blockSize==OFFSET){div=2;}else{div=7;}
            for(unsigned int h=0;h<inodeStruct.i_blocks/div;h++){
              lseek(driveFileHandle, inodeStruct.i_block[h]*blockSize, SEEK_SET);
              read(driveFileHandle, &blockholder, blockSize);
              outfile << blockholder << endl;
            }
            cout<< "Recovered -> " << pause <<endl;
            outfile.close();
          }
        }
        pause++;
      }
    }
  }
}

void Extended2::recoverInode(int inodeNum){
  unsigned int iNodeBlockCount;
  unsigned int iNodesPerGroup;
  unsigned int div;
  unsigned int pause=1;
  unsigned char blockholder[blockSize];
	string c;
	ofstream outfile;
	//inodes per block
  iNodesPerGroup=this->blockSize/sizeof(ext2_inode);
  //num of blocks in table
  iNodeBlockCount=this->superBlock.s_inodes_per_group/iNodesPerGroup;
  for(unsigned int i=0; i<this->iNodeTableLocations.size(); i++){
    for(unsigned int x=0; x<iNodeBlockCount; x++){
      for(unsigned int y=0; y<iNodesPerGroup; y++){
        if(inodeNum==pause){
          lseek(driveFileHandle, ((x+this->iNodeTableLocations.at(i))*this->blockSize)+(y*sizeof(ext2_inode)), SEEK_SET);
          read(driveFileHandle, &inodeStruct, sizeof(ext2_inode));	
          stringstream s;
          s << pause;
          c=s.str();
          outfile.open(c.c_str());
          if(blockSize==OFFSET){div=2;}else{div=7;}
          for(unsigned int h=0;h<inodeStruct.i_blocks/div;h++){
            lseek(driveFileHandle, inodeStruct.i_block[h]*blockSize, SEEK_SET);
            read(driveFileHandle, &blockholder, blockSize);
            outfile << blockholder << endl;
          }
          cout<< "Recovered -> " << pause <<endl;
          outfile.close();
        }
        pause++;
      }
    }
	}
}

void Extended2::recoverSuper(const char* path, int byteOffset){
	if((this->driveFileHandle=open(path, O_RDWR))<0){
		cout << "Failed :: cannot open " << path << endl;
		exit(-1);
	}
  lseek(driveFileHandle, byteOffset, SEEK_SET);
  read(driveFileHandle, &this->superBlock, sizeof(ext2_super_block));
  lseek(driveFileHandle, OFFSET, SEEK_SET);
  write(driveFileHandle, &this->superBlock, sizeof(ext2_super_block));
  close(driveFileHandle);
}

void Extended2::printBitmapData(int x){
		if((unsigned int)x >= this->dataBlockBitMapLocations.size() || x < 0){
      cout << "input not within range\n";
      return;
		}//end if
		char map[this->blockSize];
		if(this->blockSize == OFFSET){
      lseek(driveFileHandle,OFFSET + dataBlockBitMapLocations[x] * this->blockSize
      , SEEK_SET);
		}//end if
		else{
      lseek(driveFileHandle,dataBlockBitMapLocations[x] * this->blockSize
      , SEEK_SET);
		}//end else
		read(driveFileHandle,&map,this->blockSize);
		cout << "Data Bitmap for Block " << x << "\n";
		int low = this->blockSize * x * 8;
		int high = low + 8;
		for(unsigned int i =0; i < this->blockSize;i++){
      bitset<8> x(map[i]);
      cout << high << " - " << low << "\t\t" << x << "\n";
      low += 8;
      high += 8;
		}//end for
}//end print

void Extended2::printBitmapInode(int x){
  if((unsigned int)x >= this->iNodeBitMapLocations.size() || x < 0){
    cout << "input not within range\n";
    return;
  }//end if
  char map[this->blockSize];
  if(this->blockSize == OFFSET){
    lseek(driveFileHandle,OFFSET + iNodeBitMapLocations[x] * this->blockSize
    , SEEK_SET);
  }//end if
  else{
    lseek(driveFileHandle,iNodeBitMapLocations[x] * this->blockSize
    , SEEK_SET);
  }//end else
  read(driveFileHandle,&map,this->blockSize);
  cout << "Data Bitmap for Block " << x << "\n";
  int low = this->blockSize * x * 8;
  int high = low + 8;
  for(unsigned int i =0; i < this->blockSize;i++){
    bitset<8> x(map[i]);
    cout << high << " - " << low << "\t\t" << x << "\n";
    low += 8;
    high += 8;
  }//end for
}

void Extended2::printMeta(){
  cout << "Block Size: " << this->blockSize << "\n";
  cout << "Block Group Count: " << this->blockGroupCount << "\n";
  cout << "Block Descriptor Size: " << this->descriptorSize << "\n";
  for(unsigned int i = 0; i < this->iNodeBitMapLocations.size();i++){
      cout << "bitmap " << i << " located at: " << this->iNodeBitMapLocations[i] << "\n";
  }//end for
  for(unsigned int i = 0; i < this->dataBlockBitMapLocations.size();i++){
      cout << "bitmap " << i << " located at: " << this->dataBlockBitMapLocations[i] << "\n";
  }
}//end print meta

void Extended2::printRecover(){
	this->recoverData(0);
}//end printRecover

int Extended2::isFree(int block){
  //error checking
  if(block > this->blockSize*this->blockGroupCount){
    return -1;
  }//end if
  
  int loc, locInTable, bit;
  loc = block / (this->blockSize * 8);
  locInTable = block % this->blockSize;
  bit = block % 8;
  //cout << "loc: " << loc << endl;
  //cout << "locInTable: " << locInTable << endl;
  //cout << "bit: " << bit << endl;
  char map[this->blockSize];
  if(this->blockSize == OFFSET){
    lseek(driveFileHandle,OFFSET + iNodeBitMapLocations[loc] * this->blockSize
    , SEEK_SET);
  }//end if
  else{
    lseek(driveFileHandle,iNodeBitMapLocations[loc] * this->blockSize
    , SEEK_SET);
  }//end else
  read(driveFileHandle,&map,this->blockSize);
  char bits = map[locInTable];
  if(bit == 0){
    //do nothing
  }
  else{
    bit = 8 - bit;
  }//end else
  //cout << "bit: " << bit << endl;
  bits << bit;
  if(bits & 0b10000000){
    return 0;
  }//end if
  else{
    return 1;
  }//end else
}//end isFree
