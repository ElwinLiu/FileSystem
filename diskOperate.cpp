#include "diskOperate.h"

diskOperator::diskOperator()
{
	//创建空间
	systemStartAddr = new char[system_size * sizeof(char)];
	// 初始化盘块的位示图
	for (int i = 0; i < block_count; i++)
		systemStartAddr[i] = '0';
	//用于存放位示图的空间已被占用
	int bitMapSize = block_count * sizeof(char) / block_szie; //位示图占用盘块数:1
	for (int i = 0; i < bitMapSize; i++)                      //从零开始分配
		systemStartAddr[i] = '1';                             //盘块已被使用
}

void diskOperator::exitSystem()
{
	delete[] systemStartAddr;
}

int diskOperator::getBlock(int blockSize)
{
	int startBlock = 0;
	int sum = 0;
	for (int i = 0; i < block_count; i++)
	{
		if (systemStartAddr[i] == '0') //可用盘块
		{
			if (sum == 0) //刚开始，设置开始盘块号
				startBlock = i;
			sum++;
			if (sum == blockSize) //连续盘块是否满足需求
			{
				//满足分配，置1
				for (int j = startBlock; j < startBlock + blockSize; j++)
					systemStartAddr[j] = '1';
				return startBlock;
			}
		}
		else //已被使用,连续已经被打断
			sum = 0;
	}
	return -1;
}

char* diskOperator::getBlockAddr(int blockNum)
{
	return systemStartAddr + blockNum * block_szie; //偏移量单位为字节
}

int diskOperator::getAddrBlock(char* addr)
{
	return (addr - systemStartAddr) / block_szie;
}

int diskOperator::releaseBlock(int blockNum, int blockSize)
{
	int endBlock = blockNum + blockSize;
	//修改位示图盘块的位置为0
	for (int i = blockNum; i < endBlock; i++)
		systemStartAddr[i] = '0';
	return 0;
}
