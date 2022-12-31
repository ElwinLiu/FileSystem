#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OSDesignProj3.h"
#include <qpushbutton.h>
#include <qinputdialog.h>
#include "diskOperate.h"
#include <qstyleditemdelegate.h>
#include <qheaderview.h>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qpoint.h>
#include "myWidget.h"

struct dirUnit;
struct dirTable;
struct FCB;
struct myData;
struct myDataTable;

Q_DECLARE_METATYPE(myData)
Q_DECLARE_METATYPE(myDataTable)

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")// ��ָ���֧��VS����
#endif

#define dirTable_max_size 15 //Ŀ¼�������ֵ


class OSDesignProj3 : public QMainWindow
{
	Q_OBJECT

public:
	OSDesignProj3(QWidget* parent = Q_NULLPTR);
	//��ʼ����Ŀ¼
	void initRootDir();
	//չʾ��ǰĿ¼�ļ�
	void showDir();
	//����Ŀ¼
	int creatDir(char dirName[]);
	//�л�Ŀ¼
	int changeDir(char dirName[]);
	//ɾ��Ŀ¼
	int deleteDir(char dirName[]);
	//�޸��ļ�������Ŀ¼��
	int changeName(char oldName[], char newName[]);

	//�����ļ�
	int creatFile(char fileName[], int fileSize);
	//ɾ���ļ�
	int deleteFile(char fileName[]);

	//���ļ�
	QString read(char fileName[]);
	//���¶��ļ�
	QString reread(char fileName[], int length);
	//ִ�ж�����
	QString doRead(FCB* myFCB, int length);
	//д�ļ�����ĩβд��
	bool write(char fileName[], char content[]);
	//����д����
	bool rewrite(char fileName[], char content[]);
	//ִ��д����
	bool doWrite(FCB* myFCB, char content[]);

	//�ͷ��ļ��ڴ�
	int releaseFile(int FCBBlock);
	//���Ŀ¼��
	int addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBBlockNum);
	//����FCB
	int creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize);
	//ָ��Ŀ¼ɾ��
	int deleteFileInTable(dirTable* myDirTable, int unitIndex);
	//ɾ��Ŀ¼��
	int deleteDirUnit(dirTable* myDirTable, int unitIndex);
	//��Ŀ¼�в���Ŀ¼��Ŀ
	int findUnitInTable(dirTable* myDirTable, char unitName[]);
	void showAll(); //��TableWidget����ʾ�ü�Ŀ¼���еı���
private:
	Ui::OSDesignProj3Class ui;
	dirTable* rootDirTable;    //��Ŀ¼
	dirTable* currentDirTable; //��ǰλ��
	char path[200];            //���浱ǰ����·��
	diskOperator* Disk;
	void insert(int type, char fileName[]);
	QTreeWidgetItem* findItem(dirTable* table, char* path); //����myData, �ҵ�Ŀ��item

private slots:
	void on_click_create();
	void show_menu(QPoint);
	void click_0(); //�Ҽ����˵�0
	void click_1();	//�Ҽ����˵�1
	void slot_doubleClicked(QTableWidgetItem* it); //˫��tablewidget����
	void click_back();
	void checkself(QTreeWidgetItem* item, int c);
	void deleteItem();
	void search_path();
};

//Ŀ¼�� 64B
struct dirUnit
{
	char fileName[59]; //�ļ���
	char type;         //�ļ�����,0Ŀ¼�� 1�ļ�
	int startBlock;    //��ʼ�̿�
};

//Ŀ¼��
//һ��Ŀ¼��ռ��һ���̿飬��������ļ���Ϊ15
struct dirTable
{
	int dirUnitAmount;               //Ŀ¼����Ŀ
	dirUnit dirs[dirTable_max_size]; //Ŀ¼���б�
};

//FCB �ļ����ƿ�
struct FCB
{
	int blockNum; //�ļ�������ʼ�̿��
	int fileSize; //�ļ���С���̿�Ϊ��λ
	int dataSize; //��д������ݴ�С���ֽ�Ϊ��λ
	int readptr;  //��ָ�룬�ֽ�Ϊ��λ
	int link;     //�ļ�������
};

struct myData
{
	int dirBlock; //����Ŀ¼��(��ǰ�̿�λ��)
	char* path; //��¼���Ե�ַ
	myData(int a = -1, char p[] = NULL) { dirBlock = a; path = p; }
};

struct myDataTable
{
	int type;
	char* fileName; //��¼���Ե�ַ
	myDataTable(int t = 1, char f[] = NULL) { type = t; fileName = f; }
};

class NoFocusDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	NoFocusDelegate();
	~NoFocusDelegate();

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

void removeItem(QTreeWidgetItem*);