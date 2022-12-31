#pragma once
#define system_size 1 * 1024 * 1024        //系统大小1M
#define block_szie 1024                      //盘块大小(1k)
#define block_count system_size / block_szie //系统盘块数目(1k)

class diskOperator
{
	char* systemStartAddr; //系统起始地址
public:
	diskOperator();
	void exitSystem();
	int getBlock(int blockSize);
	char* getBlockAddr(int blockNum); //获得盘块的物理地址
	int getAddrBlock(char* addr); //获得物理地址的盘块号
	int releaseBlock(int blockNum, int blockSize); //释放盘块
};

