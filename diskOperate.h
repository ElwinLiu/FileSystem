#pragma once
#define system_size 1 * 1024 * 1024        //ϵͳ��С1M
#define block_szie 1024                      //�̿��С(1k)
#define block_count system_size / block_szie //ϵͳ�̿���Ŀ(1k)

class diskOperator
{
	char* systemStartAddr; //ϵͳ��ʼ��ַ
public:
	diskOperator();
	void exitSystem();
	int getBlock(int blockSize);
	char* getBlockAddr(int blockNum); //����̿�������ַ
	int getAddrBlock(char* addr); //��������ַ���̿��
	int releaseBlock(int blockNum, int blockSize); //�ͷ��̿�
};

