#include "ext2_fs.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>

#define OFFSET 1024
using namespace std;

class DeviceMapper{
    private:
        struct ext2_super_block superBlock;
        struct ext2_group_desc descriptorBlock;
        struct ext2_inode inodeStruct;
        struct ext2_dir_entry *entry;
        unsigned char* blockBitmap;
        unsigned char* inodeTable;
        unsigned int blockSize;
        unsigned int blockGroupCount;
        unsigned int descriptorSize;
        int driveHandle;
        vector<int> blockBitMapLocations;
        vector<int> iNodeBitMapLocations;
        vector<int> iNodeTableLocations;
        struct tm *ptm;

    public:
        DeviceMapper(const char* pathToFile, int bad){
            if((driveHandle=open(pathToFile, O_RDONLY))<0){
                    cout << "failed to open " << pathToFile << endl;
                    exit(1);
                }
            if(!bad){
                lseek(driveHandle, OFFSET, SEEK_SET);
                read(driveHandle, &superBlock, sizeof(ext2_super_block));
                if(superBlock.s_magic != EXT2_SUPER_MAGIC){
                    cout << "failed :: bad magic number " << superBlock.s_magic << endl;
                    exit(1);
                }
                //get block size, probably 1024
                blockSize = OFFSET << superBlock.s_log_block_size;
                //calculate number of block groups on disk
                blockGroupCount = 1+(superBlock.s_blocks_count-1)/superBlock.s_blocks_per_group;
                //calculate number of bytes for group descriptor
                descriptorSize = blockGroupCount*sizeof(ext2_group_desc);
            }else{
                //find backup superblock here
                cout << "bad" <<endl;
            }
        }

        void bestGuess(){
            int x;
            int z=superBlock.s_blocks_count*blockSize;
            for(int i=520623104;i<z;i++){
                lseek(driveHandle, i, SEEK_SET);
                read(driveHandle, &x, 1);
                cout<<x;
            }
        }

        void printiNodeBitMaps(){
            int number=0;
            blockBitmap=(unsigned char*)malloc(blockSize);
            for(unsigned int i=0;i<this->blockBitMapLocations.size();i++){
                lseek(driveHandle, OFFSET+((this->blockBitMapLocations.at(i))*this->blockSize), SEEK_SET);
                read(driveHandle, blockBitmap, blockSize);
                for(unsigned int y=0;y<blockSize;y++){
                    if(!(blockBitmap[y]==0)){
                        if(blockBitmap[y] & 0x80){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x40){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x20){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x10){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x8){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x4){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x2){cout<<"1";}else{cout<<"0";}
                        if(blockBitmap[y] & 0x1){cout<<"1";}else{cout<<"0";}
                        cout <<endl;
                        /*
                        if(blockBitmap[y] & 0x80){cout<<"Allocated at bit offset: "<<number<< endl;}
                        if(blockBitmap[y] & 0x40){cout<<"Allocated at bit offset: "<<number+1<<endl;}
                        if(blockBitmap[y] & 0x20){cout<<"Allocated at bit offset: "<<number+2<<endl;}
                        if(blockBitmap[y] & 0x10){cout<<"Allocated at bit offset: "<<number+3<<endl;}
                        if(blockBitmap[y] & 0x8){cout<<"Allocated at bit offset: "<<number+4<<endl;}
                        if(blockBitmap[y] & 0x4){cout<<"Allocated at bit offset: "<<number+5<<endl;}
                        if(blockBitmap[y] & 0x2){cout<<"Allocated at bit offset: "<<number+6<<endl;}
                        if(blockBitmap[y] & 0x1){cout<<"Allocated at bit offset: "<<number+7<<endl;}
                        */
                    }
                    number+=8;
                }
            }
            free(blockBitmap);
            cout<<endl;
        }

        void printGroupDesc(){
            if(blockSize==4096){
                lseek(driveHandle, this->blockSize, SEEK_SET);

            }else{
                lseek(driveHandle, OFFSET+this->blockSize, SEEK_SET);
            }
            for(unsigned int i=0;i<this->blockGroupCount;i++){
                read(driveHandle, &descriptorBlock, sizeof(ext2_group_desc));
                cout << "GROUP DESCRIPTOR :: " << i << " ::"<< endl;
                cout << "free blocks: " << descriptorBlock.bg_free_blocks_count << endl;
                cout << "free inode count: " << descriptorBlock.bg_free_inodes_count << endl;
                cout << "inode bitmap: " << descriptorBlock.bg_inode_bitmap << endl;
                cout << "inode table: " << descriptorBlock.bg_inode_table << endl;
                cout << "Block bitmap: " << descriptorBlock.bg_block_bitmap << endl;
                cout << "directories: " << descriptorBlock.bg_used_dirs_count << endl;
                this->iNodeBitMapLocations.push_back(descriptorBlock.bg_inode_bitmap);
                this->iNodeTableLocations.push_back(descriptorBlock.bg_inode_table);
                this->blockBitMapLocations.push_back(descriptorBlock.bg_block_bitmap);
            }
            cout << endl;
        }

        void printSuperBlock(){
            cout << "Block Count-> " << this->superBlock.s_blocks_count <<endl;
            cout << "Block Size-> " << this->blockSize<<endl;
            cout << "Blocks Per Group-> " << this->superBlock.s_blocks_per_group <<endl;
            cout << "Free Blocks-> "<< this->superBlock.s_free_blocks_count <<endl;
            cout << "Free Inodes-> "<< this->superBlock.s_free_inodes_count <<endl;
            cout << "Inode Count-> "<< this->superBlock.s_inodes_count <<endl;
            cout << "Inodes Per Group-> "<< this->superBlock.s_inodes_per_group <<endl;
            cout << "First inode-> "<< this->superBlock.s_first_ino <<endl;
            cout << "Magic Number-> "<< this->superBlock.s_magic <<endl;
        }

        void printInode(){
            map<int, ext2_inode> nodeMap;
            map<int, int> inodeMap;
            unsigned int iNodeBlockCount;
            unsigned int iNodesPerGroup;
            unsigned int div;
            unsigned int optnum=1;
            unsigned char blockHolder[blockSize];
            unsigned int pause=1;
            unsigned int s=0;
            unsigned char blockholder[blockSize];
            time_t tim;
            char text[25];
            iNodesPerGroup=this->blockSize/sizeof(ext2_inode);
            iNodeBlockCount=this->superBlock.s_inodes_per_group/iNodesPerGroup;
            for(unsigned int i=0; i<this->iNodeTableLocations.size(); i++){
                for(unsigned int x=0; x<iNodeBlockCount; x++){
                    for(unsigned int y=0; y<iNodesPerGroup; y++){
                        lseek(driveHandle, ((x+this->iNodeTableLocations.at(i))*this->blockSize)+(y*sizeof(ext2_inode)), SEEK_SET);
                        read(driveHandle, &inodeStruct, sizeof(ext2_inode));

                        if(S_ISDIR(inodeStruct.i_mode)){
                            cout<< "dir"<<endl;
                            cout<< "blocks: "<<inodeStruct.i_blocks/2 <<endl;
                            cout<< "links: "<<inodeStruct.i_links_count <<endl;
                            cout<< "size: "<<inodeStruct.i_size <<endl;
                            cout<< "inode #: "<<pause <<endl;
                            if(this->blockSize==OFFSET){div=2;}else{div=7;}
                            for(int p=0;p<(inodeStruct.i_blocks/div);p++){
                                cout << "blocks: "<<inodeStruct.i_block[p] <<endl;
                                lseek(driveHandle, blockSize*inodeStruct.i_block[p],SEEK_SET);
                                read(driveHandle, blockHolder, blockSize);
                                entry=(struct ext2_dir_entry*)blockHolder;
                                //while(s<inodeStruct.i_size){
                                //    char filename[EXT2_NAME_LEN+1];
                                //    memcpy(filename, entry->name, entry->name_len);
                                //    filename[entry->name_len]+0;
                                //    printf("%10u : %s\n", entry->inode,filename);
                                //    entry=entry + entry->rec_len;
                                //    s+=entry->rec_len;
                                //}
                            }
                            cout<<endl;
                        }

                        /*
                        if(S_ISREG(inodeStruct.i_mode)){cout<< "reg"<<endl;
                                cout <<"blocks: "<<inodeStruct.i_blocks/2<<endl;
                                cout <<"size: "<<inodeStruct.i_size<<endl;
                                cout <<"links: "<<inodeStruct.i_links_count<<endl;
                        }
                        */
                        if(!this->inodeStruct.i_dtime==0){
                            /*
                            tim=inodeStruct.i_dtime;
                            ptm=gmtime(&tim);
                            cout << "delete time: " << ptm->tm_hour%12<<":"<<ptm->tm_min<< " "<<ptm->tm_year+1900 << endl;
                            cout << "block count: " << this->inodeStruct.i_blocks/2 << endl;
                            cout << "size: " << this->inodeStruct.i_size << endl;
                            */
                            cout << optnum << "  |\tinode #: "<< pause << endl;
                            nodeMap[optnum]=inodeStruct;
                            inodeMap[optnum]=pause;
                            //for(int g=0;g<15;g++){
                            //    cout << "blocks: \t"<<this->inodeStruct.i_block[g] <<endl;
                            //}
                            //cout<<endl;
                            optnum++;
                        }
                        pause++;
                    }
                }
            }
            cin >> optnum;
            //cout << nodeMap[optnum].i_block[0] <<endl;
            ofstream outfile;
            s=inodeMap[optnum];
            //ostringstream oss;
            //oss << s << ".txt";
            //outfile.open(oss.str());
            //outfile.open(to_string(s));
            if(!outfile){
                exit(-1);
            }
            for(int h=0;h<nodeMap[optnum].i_blocks/2;h++){
                lseek(driveHandle, nodeMap[optnum].i_block[h]*blockSize, SEEK_SET);
                read(driveHandle, &blockholder, blockSize);
                outfile << blockholder << endl;
            }




        }



};

int main(){
    int in;
    DeviceMapper device=DeviceMapper("/dev/sdb1", 0);
    device.printSuperBlock();
    //DeviceMapper s=DeviceMapper("/dev/sdb1",0);
    //DeviceMapper d=DeviceMapper("/dev/sdb1",1);
    device.printGroupDesc();
    //cin >> in;
    device.printiNodeBitMaps();
    //device.printInode();
    //device.bestGuess();
    //s.printSuperBlock();
	return 0;

}
