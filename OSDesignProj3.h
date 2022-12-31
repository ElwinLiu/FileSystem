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
#pragma execution_character_set("utf-8")// 该指令仅支持VS环境
#endif

#define dirTable_max_size 15 //目录表项最大值


class OSDesignProj3 : public QMainWindow
{
	Q_OBJECT

public:
	OSDesignProj3(QWidget* parent = Q_NULLPTR);
	//初始化根目录
	void initRootDir();
	//展示当前目录文件
	void showDir();
	//创建目录
	int creatDir(char dirName[]);
	//切换目录
	int changeDir(char dirName[]);
	//删除目录
	int deleteDir(char dirName[]);
	//修改文件名或者目录名
	int changeName(char oldName[], char newName[]);

	//创建文件
	int creatFile(char fileName[], int fileSize);
	//删除文件
	int deleteFile(char fileName[]);

	//读文件
	QString read(char fileName[]);
	//重新读文件
	QString reread(char fileName[], int length);
	//执行读操作
	QString doRead(FCB* myFCB, int length);
	//写文件，从末尾写入
	bool write(char fileName[], char content[]);
	//重新写覆盖
	bool rewrite(char fileName[], char content[]);
	//执行写操作
	bool doWrite(FCB* myFCB, char content[]);

	//释放文件内存
	int releaseFile(int FCBBlock);
	//添加目录项
	int addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBBlockNum);
	//创建FCB
	int creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize);
	//指定目录删除
	int deleteFileInTable(dirTable* myDirTable, int unitIndex);
	//删除目录项
	int deleteDirUnit(dirTable* myDirTable, int unitIndex);
	//从目录中查找目录项目
	int findUnitInTable(dirTable* myDirTable, char unitName[]);
	void showAll(); //在TableWidget中显示该级目录所有的表项
private:
	Ui::OSDesignProj3Class ui;
	dirTable* rootDirTable;    //根目录
	dirTable* currentDirTable; //当前位置
	char path[200];            //保存当前绝对路径
	diskOperator* Disk;
	void insert(int type, char fileName[]);
	QTreeWidgetItem* findItem(dirTable* table, char* path); //利用myData, 找到目标item

private slots:
	void on_click_create();
	void show_menu(QPoint);
	void click_0(); //右键，菜单0
	void click_1();	//右键，菜单1
	void slot_doubleClicked(QTableWidgetItem* it); //双击tablewidget表项
	void click_back();
	void checkself(QTreeWidgetItem* item, int c);
	void deleteItem();
	void search_path();
};

//目录项 64B
struct dirUnit
{
	char fileName[59]; //文件名
	char type;         //文件类型,0目录， 1文件
	int startBlock;    //起始盘块
};

//目录表
//一个目录表占用一个盘块，允许最多文件数为15
struct dirTable
{
	int dirUnitAmount;               //目录项数目
	dirUnit dirs[dirTable_max_size]; //目录项列表
};

//FCB 文件控制块
struct FCB
{
	int blockNum; //文件数据起始盘块号
	int fileSize; //文件大小，盘块为单位
	int dataSize; //已写入的内容大小，字节为单位
	int readptr;  //读指针，字节为单位
	int link;     //文件链接数
};

struct myData
{
	int dirBlock; //所属目录表(当前盘块位置)
	char* path; //记录绝对地址
	myData(int a = -1, char p[] = NULL) { dirBlock = a; path = p; }
};

struct myDataTable
{
	int type;
	char* fileName; //记录绝对地址
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