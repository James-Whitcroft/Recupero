/* this class does all the heavy lifting for recupero.
This class will read, parse and output everything that is called.
@author: samuel Launt
@author: James 

*/
#ifndef EXTENDED2_H_
#define EXTENDED2_H_

#include "ext2_fs.h"
#include <vector>
using std::vector;
//used to skip boot block
#define OFFSET 1024

class Extended2{
  private:

    /* This method will read the superblock and do math for ext2 things
    @parm: path to device

    */
    void meta(const char* path);

    /* this method will check to see if a data block is free
    @parm: int - block num
    @post: int - 1 yes the block is free 0 no the block is not free
    -1 for error!
    NOTE: there is no check on the input!!
    */
    int isFree(int block);

    /*
    structs for blocks
    */
    struct ext2_super_block superBlock;
    struct ext2_group_desc descriptorBlock;
    struct ext2_inode inodeStruct;
    struct ext2_dir_entry dirEntryStruct;
    
    /*
    results of open(path_to_file) stored here
    */
    int driveFileHandle;
    
    /*
    math done by meta stored here
    */
    unsigned int blockSize;
    unsigned int blockGroupCount;
    unsigned int descriptorSize;
    unsigned char* inodeBitMap;
    unsigned char* blockBitMap;
    
    /*
    bit map locations
    push_back() for insert
    pop_back() for remove	
    */
    vector<int> iNodeBitMapLocations;
    vector<int> dataBlockBitMapLocations;
    vector<int> iNodeTableLocations;
    
  public:

    /* This is the constructor
    @param: char* - path to partition with ext2 filesystem
    */
    Extended2(const char* path);

    /*other constructor. this is for when recovering superblock
    @parm: int - this is the block size of the file system

    */
    Extended2(const char* path,int blkOffset);

    /*destructor, mainly to close driveFileHandle
    */
    ~Extended2();

    /* This method prints the inode info
    @parm: int - inode number

    */
    void printInode(unsigned int num);

    /* This method prints the super block info

    */
    void printSuper();

    /* This method prints the group descriptors

    */
    void printGroup();

    /* This method will try to recover a superblock by overwritting 
    the main with a backup.

    */
    void recoverSuper(const char*, int);

    /* This method will try to recover data that was once deleted
    int x - 0=print recoverable inodes to stdout; 1=recover all data, no print
    */
    void recoverData(int x);

    /*this method will recover individual files given inode
    int x - the inode associated with file
    */        
    void recoverInode(int x);

    /* this method will print the inodes that are recoverable
      calls recoverData(0)
    */
    void printRecover();

    /* this method will print the databit #x
    int x - the number of bitmap to print

    */
    void printBitmapData(int x);

    /* this method will print the inode bitmap #x
    int x- the number of bitmap to print

    */
    void printBitmapInode(int x);

    /* this method will print the vaous metadata needed to run program

    */
    void printMeta();

};//end class

#endif
