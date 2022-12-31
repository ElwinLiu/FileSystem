#include "OSDesignProj3.h"

char* toChar(QString str)
{
	char* ch;
	QByteArray ba = str.toLatin1();
	ch = ba.data();
	ch[str.size()] = '\0';
	return ch;
}

OSDesignProj3::OSDesignProj3(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//QStandardItemModel* model_ = new QStandardItemModel(ui.tableWidge);
	//model_->setHorizontalHeaderLabels(QStringList() <<
	//	QStringLiteral("名称") << QStringLiteral("类型")
	//	<< QStringLiteral("修改日期"));
	//ui.tableWidget->setModel(model_);
	ui.tableWidget->verticalHeader()->hide(); //隐藏图标
	QStringList strs = { "名称", "类型" }; //列标题
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(15);
	ui.tableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(204,232,255) }");//改变选中的颜色
	ui.tableWidget->setItemDelegate(new NoFocusDelegate()); //设置点击的表格无虚线
	ui.tableWidget->setColumnCount(2); //设置列数
	ui.tableWidget->setHorizontalHeaderLabels(strs); //设置列标题
	//ui.tableWidget->horizontalHeader()->setStretchLastSection(true); //列占满
	ui.tableWidget->setShowGrid(false); //隐藏网格
	ui.tableWidget->setSelectionBehavior(QTableWidget::SelectRows); //一次选择一行
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu); //右键准备
	//初始化系统
	Disk = new diskOperator;
	//创建根目录
	initRootDir();
	connect(ui.tableWidget, SIGNAL(customContextMenuRequested(QPoint))
		, this, SLOT(show_menu(QPoint)));//右键显示菜单
	connect(ui.actionCreate_user, SIGNAL(triggered()), this, SLOT(on_click_create()));
	connect(ui.tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
		this, SLOT(slot_doubleClicked(QTableWidgetItem*)));
	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(click_back()));
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(checkself(QTreeWidgetItem*, int)));//检测目录结构的点击事件，信号槽机制
	connect(ui.pushButton_3, SIGNAL(clicked(bool)), this, SLOT(deleteItem()));
	connect(ui.pushButton_2, SIGNAL(clicked(bool)), this, SLOT(search_path()));
}

void OSDesignProj3::initRootDir()
{
	Disk = new diskOperator;
	//分配一个盘块空间给rootDirTable
	int startBlock = Disk->getBlock(1);
	if (startBlock == -1)
		return;
	rootDirTable = (dirTable*)Disk->getBlockAddr(startBlock);
	rootDirTable->dirUnitAmount = 0;
	//将自身作为父级目录
	//addDirUnit(rootDirTable, "..", 0, startBlock);

	currentDirTable = rootDirTable;
	//初始化初始绝对路径
	path[0] = '\\';
	path[1] = '\0';
}

void OSDesignProj3::showDir()
{
	int unitAmount = currentDirTable->dirUnitAmount;
	printf("total %d\n", unitAmount);
	printf("name\ttype\tsize\tFCB\tdataStartBlock\n");
	//遍历所有表项
	for (int i = 0; i < unitAmount; i++)
	{
		//获取目录项
		dirUnit unitTemp = currentDirTable->dirs[i];
		printf("%s\t%d\t", unitTemp.fileName, unitTemp.type);
		//该表项是文件，继续输出大小和起始盘块号
		if (unitTemp.type == 1)
		{
			int FCBBlock = unitTemp.startBlock;
			FCB* fileFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
			printf("%d\t%d\t%d\n", fileFCB->fileSize, FCBBlock, fileFCB->blockNum);
		}
		else
		{
			int dirBlock = unitTemp.startBlock;
			dirTable* myTable = (dirTable*)Disk->getBlockAddr(dirBlock);
			printf("%d\t%d\n", myTable->dirUnitAmount, unitTemp.startBlock);
		}
	}
}

int OSDesignProj3::creatDir(char dirName[])
{
	if (strlen(dirName) >= 59)
	{
		printf("file name too long\n");
		return -1;
	}
	//为目录表分配空间
	int dirBlock = Disk->getBlock(1);
	if (dirBlock == -1)
		return -1;
	//将目录作为目录项添加到当前目录
	if (addDirUnit(currentDirTable, dirName, 0, dirBlock) == -1)
		return -1;
	//为新建的目录添加一个到父目录的目录项
	dirTable* newTable = (dirTable*)Disk->getBlockAddr(dirBlock);
	newTable->dirUnitAmount = 0;
	char parent[] = "..";
	if (addDirUnit(newTable, parent, 0, Disk->getAddrBlock((char*)currentDirTable)) == -1)
		return -1;
	return 0;
}

int OSDesignProj3::changeDir(char dirName[])
{
	//目录项在目录位置
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	//不存在
	if (unitIndex == -1)
	{
		printf("cd: no such file or directory: %s\n", dirName);
		return -1;
	}
	if (currentDirTable->dirs[unitIndex].type == 1)
	{
		printf("cd: not a directory: %s\n", dirName);
		return -1;
	}
	//修改当前目录
	int dirBlock = currentDirTable->dirs[unitIndex].startBlock;
	currentDirTable = (dirTable*)Disk->getBlockAddr(dirBlock); // 当前盘块位置
	//修改全局绝对路径
	if (strcmp(dirName, "..") == 0)
	{
		//回退绝对路径
		int len = strlen(path);
		for (int i = len - 2; i >= 0; i--)
			if (path[i] == '\\')
			{
				path[i + 1] = '\0';
				break;
			}
	}
	else
	{
		strcat(path, dirName);
		strcat(path, "\\");
	}

	return 0;
}

int OSDesignProj3::deleteDir(char dirName[])
{
	//忽略系统的自动创建的父目录
	if (strcmp(dirName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//查找文件
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0) //目录
	{
		deleteFileInTable(currentDirTable, unitIndex);
	}
	else
	{
		printf("not a dir\n");
		return -1;
	}
	//从目录表中剔除
	deleteDirUnit(currentDirTable, unitIndex);
	return 0;
}

int OSDesignProj3::changeName(char oldName[], char newName[])
{
	int unitIndex = findUnitInTable(currentDirTable, oldName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	strcpy(currentDirTable->dirs[unitIndex].fileName, newName);
	return 0;
}

int OSDesignProj3::creatFile(char fileName[], int fileSize)
{
	//检测文件名字长度
	if (strlen(fileName) >= 59)
	{
		printf("file name too long\n");
		return -1;
	}
	//获得FCB的空间
	int FCBBlock = Disk->getBlock(1);
	if (FCBBlock == -1)
		return -1;
	//获取文件数据空间
	int FileBlock = Disk->getBlock(fileSize);
	if (FileBlock == -1)
		return -1;
	//创建FCB
	if (creatFCB(FCBBlock, FileBlock, fileSize) == -1)
		return -1;
	//添加到目录项
	if (addDirUnit(currentDirTable, fileName, 1, FCBBlock) == -1)
		return -1;

	return 0;
}

int OSDesignProj3::deleteFile(char fileName[])
{
	//忽略系统的自动创建的父目录
	if (strcmp(fileName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//查找文件的目录项位置
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0) //目录
	{
		printf("not a file\n");
		return -1;
	}
	int FCBBlock = myUnit.startBlock;
	//释放内存
	releaseFile(FCBBlock);
	//从目录表中剔除
	deleteDirUnit(currentDirTable, unitIndex);
	return 0;
}

QString OSDesignProj3::read(char fileName[])
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file no found\n");
		return -1;
	}
	//控制块
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	return reread(fileName, myFCB->dataSize); //读取所有数据
}

QString OSDesignProj3::reread(char fileName[], int length)
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file no found\n");
		return -1;
	}
	//控制块
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	myFCB->readptr = 0;
	return doRead(myFCB, length);
}

QString OSDesignProj3::doRead(FCB* myFCB, int length)
{
	QString store = "";//存储数据
	//读数据
	int dataSize = myFCB->dataSize;
	char* data = (char*)Disk->getBlockAddr(myFCB->blockNum);
	//在不超出数据长度下，读取指定长度的数据
	for (int i = 0; i < length && myFCB->readptr < dataSize; i++, myFCB->readptr++)
	{
		printf("%c", *(data + myFCB->readptr));
		store = store + *(data + myFCB->readptr);
	}
	return store;
}

bool OSDesignProj3::write(char fileName[], char content[])
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file no found\n");
		return false;
	}
	//控制块
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	return doWrite(myFCB, content);
}

bool OSDesignProj3::rewrite(char fileName[], char content[])
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file no found\n");
		return false;
	}
	//控制块
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	//重设数据块
	myFCB->dataSize = 0;
	myFCB->readptr = 0;

	return doWrite(myFCB, content);

}

bool OSDesignProj3::doWrite(FCB* myFCB, char content[])
{
	int contentLen = strlen(content);
	int fileSize = myFCB->fileSize * block_szie;
	char* data = (char*)Disk->getBlockAddr(myFCB->blockNum);
	//在不超出文件的大小的范围内写入
	for (int i = 0; i < contentLen && myFCB->dataSize < fileSize; i++, myFCB->dataSize++)
	{
		*(data + myFCB->dataSize) = content[i];
	}
	if (myFCB->dataSize == fileSize)
		return false;
	return true;
}

int OSDesignProj3::releaseFile(int FCBBlock)
{
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	myFCB->link--; //链接数减一
	//无链接，删除文件
	if (myFCB->link == 0)
	{
		//释放文件的数据空间
		Disk->releaseBlock(myFCB->blockNum, myFCB->fileSize);
	}
	//释放FCB的空间
	Disk->releaseBlock(FCBBlock, 1);
	return 0;
}

int OSDesignProj3::addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBBlockNum)
{
	//获得目录表
	int dirUnitAmount = myDirTable->dirUnitAmount;
	//检测目录表示是否已满
	if (dirUnitAmount == dirTable_max_size)
	{
		printf("dirTables is full, try to delete some file\n");
		return -1;
	}

	//是否存在同名文件
	if (findUnitInTable(myDirTable, fileName) != -1)
	{
		printf("mkdir: %s: File exists\n", fileName);
		return -1;
	}
	//构建新目录项
	dirUnit* newDirUnit = &myDirTable->dirs[dirUnitAmount];
	myDirTable->dirUnitAmount++; //当前目录表的目录项数量+1
	//设置新目录项内容
	strcpy(newDirUnit->fileName, fileName);
	newDirUnit->type = type;
	newDirUnit->startBlock = FCBBlockNum;

	return 0;
}

int OSDesignProj3::creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize)
{
	//找到fcb的存储位置
	FCB* currentFCB = (FCB*)Disk->getBlockAddr(fcbBlockNum);
	currentFCB->blockNum = fileBlockNum; //文件数据起始位置
	currentFCB->fileSize = fileSize;     //文件大小
	currentFCB->link = 1;                //文件链接数
	currentFCB->dataSize = 0;            //文件已写入数据长度
	currentFCB->readptr = 0;             //文件读指针
	return 0;
}

int OSDesignProj3::deleteFileInTable(dirTable* myDirTable, int unitIndex)
{
	//查找文件
	dirUnit myUnit = myDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0) //目录
	{
		//找到目录位置
		int FCBBlock = myUnit.startBlock;
		dirTable* table = (dirTable*)Disk->getBlockAddr(FCBBlock);
		//递归删除目录下的所有文件
		printf("cycle delete dir %s\n", myUnit.fileName);
		int unitCount = table->dirUnitAmount;
		for (int i = 1; i < unitCount; i++) //为什么？？？忽略“..”
		{
			printf("delete %s\n", table->dirs[i].fileName);
			deleteFileInTable(table, i);
		}
		//释放目录表空间
		Disk->releaseBlock(FCBBlock, 1);
	}
	else
	{ //文件
		//释放文件内存
		int FCBBlock = myUnit.startBlock;
		releaseFile(FCBBlock);
	}
	return 0;
}

int OSDesignProj3::deleteDirUnit(dirTable* myDirTable, int unitIndex)
{
	//迁移覆盖
	int dirUnitAmount = myDirTable->dirUnitAmount;
	for (int i = unitIndex; i < dirUnitAmount - 1; i++)
	{
		myDirTable->dirs[i] = myDirTable->dirs[i + 1];
	}
	myDirTable->dirUnitAmount--;
	return 0;
}

int OSDesignProj3::findUnitInTable(dirTable* myDirTable, char unitName[])
{
	//获得目录表
	int dirUnitAmount = myDirTable->dirUnitAmount;
	int unitIndex = -1;
	for (int i = 0; i < dirUnitAmount; i++) //查找目录项位置
		if (strcmp(unitName, myDirTable->dirs[i].fileName) == 0)
			unitIndex = i;
	return unitIndex;
}

//在TableWidget中显示该级目录所有的表项
void OSDesignProj3::showAll()
{
	ui.tableWidget->clearContents();//清空表
	ui.tableWidget->setRowCount(0);	int unitAmount = currentDirTable->dirUnitAmount;
	//遍历所有表项
	for (int i = 0; i < unitAmount; i++)
	{
		if (strcmp(currentDirTable->dirs[i].fileName, "..") == 0) continue;
		//获取目录项
		char* name = new char;
		strcpy(name, currentDirTable->dirs[i].fileName);
		QTableWidgetItem* btItem = new QTableWidgetItem();   //图标 + 文件名 
		btItem->setTextAlignment(Qt::AlignCenter);			//文字居中
		QString t;
		if (currentDirTable->dirs[i].type == 0)
		{//为目录类型
			btItem->setIcon(QIcon(":/OSDesignProj3/folder.png"));
			t = "文件夹";
		}
		else
		{//为文件类型
			QTreeWidgetItem* item = findItem(currentDirTable, path);
			btItem->setIcon(QIcon(":/OSDesignProj3/file.png"));
			t = "文件";
		}
		strcpy(name, currentDirTable->dirs[i].fileName);
		myDataTable dataTable(currentDirTable->dirs[i].type, name);
		btItem->setData(Qt::UserRole, QVariant::fromValue(dataTable));
		btItem->setText(currentDirTable->dirs[i].fileName);
		int RowCont; //当前要写第几行
		RowCont = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(RowCont);//增加一行
		ui.tableWidget->setItem(RowCont, 0, btItem);
		ui.tableWidget->setItem(RowCont, 1, new QTableWidgetItem(t));
	}
}

//添加文件/文件夹
void OSDesignProj3::insert(int type, char fileName[])
{
	QTableWidgetItem* btItem = new QTableWidgetItem();   //图标 + 文件名 
	btItem->setTextAlignment(Qt::AlignCenter);			//文字居中
	QString t;
	if (type == 0)
	{//为目录类型
		btItem->setIcon(QIcon(":/OSDesignProj3/folder.png"));
		t = "文件夹";
		/*如果加的是文件夹 ，那么在目录结构中要显示该文件夹，在表中也要显示。*/
		creatDir(fileName);
		QTreeWidgetItem* item = findItem(currentDirTable, path);
		if (currentDirTable->dirUnitAmount >= 15) return;
		QString name = fileName;
		QTreeWidgetItem* new_item = new QTreeWidgetItem(item, QStringList(name));
		char* new_path = new char[200];
		//存的表指针指向当前目录自己的表
		changeDir(fileName);
		strcpy(new_path, path);
		myData data(Disk->getAddrBlock((char*)currentDirTable), new_path);

		//返回父目录
		char temp[] = "..";
		changeDir(temp);
		new_item->setData(0, Qt::UserRole + 1, QVariant::fromValue(data));
		//
		int test = 1;
		//
	}
	else
	{//为文件类型
		QTreeWidgetItem* item = findItem(currentDirTable, path);
		if (currentDirTable->dirUnitAmount >= 15) return;
		btItem->setIcon(QIcon(":/OSDesignProj3/file.png"));
		t = "文件";
		creatFile(fileName, 100);
	}
	//记录表项属性
	char* name = new char;
	strcpy(name, fileName);
	myDataTable dataTable(type, name);
	btItem->setData(Qt::UserRole, QVariant::fromValue(dataTable));
	btItem->setText(fileName);
	int RowCont; //当前要写第几行
	RowCont = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(RowCont);//增加一行
	ui.tableWidget->setItem(RowCont, 0, btItem);
	ui.tableWidget->setItem(RowCont, 1, new QTableWidgetItem(t));
}

QTreeWidgetItem* OSDesignProj3::findItem(dirTable* table, char* path)
{
	//遍历treeWidget
	QTreeWidgetItemIterator it(ui.treeWidget);
	while (*it) {
		myData data = (*it)->data(0, Qt::UserRole + 1).value<myData>(); //遍历树的item属性
		int tmp = Disk->getAddrBlock((char*)table);
		if (strcmp(data.path, path) == 0 && data.dirBlock == tmp)
		{
			return *it;
		}
		++it;
	}
}

void OSDesignProj3::on_click_create()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("创建用户"), tr("请输入用户名"), QLineEdit::Normal, 0, &ok);
	if (ok && !text.isEmpty())
	{
		dirTable* temp = currentDirTable;
		currentDirTable = rootDirTable;
		char* ch;
		QByteArray ba = text.toLocal8Bit();
		ch = ba.data();
		char name[59] = {};
		strcpy(name, ch);
		char root_path[] = "\\";
		char temp_path[] = "temp";
		strcpy(temp_path, path);
		strcpy(path, root_path);
		if (creatDir(name) == -1)
		{//出错，退回去
			currentDirTable = temp;
			strcpy(path, temp_path);
			return;
		}
		changeDir(name); //自动进入该目录
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.treeWidget, QStringList(text));
		char* new_path = new char;
		strcpy(new_path, path);
		myData data(Disk->getAddrBlock((char*)currentDirTable), new_path);
		item->setData(0, Qt::UserRole + 1, QVariant::fromValue(data));
		ui.textEdit->setText(QString(path));
	}

}

void OSDesignProj3::show_menu(QPoint)
{
	//设置菜单选项
	QMenu* menu = new QMenu(ui.tableWidget);
	QAction* pnew = new QAction("添加文件", ui.tableWidget);
	QAction* pnew1 = new QAction("添加目录", ui.tableWidget);
	connect(pnew, SIGNAL(triggered()), this, SLOT(click_0()));
	connect(pnew1, SIGNAL(triggered()), this, SLOT(click_1()));
	menu->addAction(pnew);
	menu->addAction(pnew1);
	menu->move(cursor().pos());
	menu->show();
	////获得鼠标点击的x，y坐标点
	//int x = pos().x();
	//int y = pos().y();
	//QModelIndex index = ui.tableWidget->indexAt(QPoint(x, y));
	//int row = index.row();//获得QTableWidget列表点击的行数
}

//添加文件
void OSDesignProj3::click_0()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("创建文件"), tr("请输入文件名"), QLineEdit::Normal, 0, &ok);
	if (ok && !text.isEmpty())
	{
		std::string str = text.toStdString();
		const char* ch = str.data();
		char name[59] = {};
		strcpy(name, ch);
		insert(1, name);
	}
}

//添加目录
void OSDesignProj3::click_1()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("创建文件夹"), tr("请输入文件夹名"), QLineEdit::Normal, 0, &ok);
	if (ok && !text.isEmpty())
	{
		std::string str = text.toStdString();
		const char* ch = str.data();
		char name[59] = {};
		strcpy(name, ch);
		insert(0, name);
	}
}

void OSDesignProj3::slot_doubleClicked(QTableWidgetItem* tmp_item)
{
	int row = tmp_item->row();
	myDataTable data = ui.tableWidget->item(row, 0)->data(Qt::UserRole).value<myDataTable>();
	if (data.type == 0)
	{//如果是目录
		ui.tableWidget->clearContents();
		ui.tableWidget->setRowCount(0);
		changeDir(data.fileName); //进入该目录
		int unitAmount = currentDirTable->dirUnitAmount;
		showAll();
		ui.textEdit->setText(QString(path));
	}
	else
	{//如果双击文件
		QMessageBox msg;
		msg.setText("插入信息？");
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		auto res = msg.exec();

		if (res == QMessageBox::Yes)
		{//插入文本信息
			bool ok;
			QString text = QInputDialog::getText(this, tr("插入文本信息"), tr("请输入"), QLineEdit::Normal, 0, &ok);
			if (ok && !text.isEmpty())
			{
				char* ch = new char;
				QByteArray ba = text.toLocal8Bit();
				ch = ba.data();
				if (write(data.fileName, ch)) return;
				else QMessageBox::about(this, "注意", "长度过长，超出部分已省略");
			}
		}
		else if (res == QMessageBox::No)
		{
			//新建窗口对象 
			QString text = read(data.fileName);
			QWidget* qw = new QWidget();
			qw->resize(800, 800);
			//新建QTextEdit对象，父窗口为qw
			QTextEdit* qte = new QTextEdit(text, qw);
			qte->setDisabled(true);
			qte->setGeometry(0, 0, 800, 800);
			qw->show();
		}

	}

}

void OSDesignProj3::click_back()
{
	char ch[] = "..";
	changeDir(ch);
	showAll();
}

void OSDesignProj3::checkself(QTreeWidgetItem* item, int c)
{
	myData data = item->data(0, Qt::UserRole + 1).value<myData>(); //遍历树的item属性
	currentDirTable = (dirTable*)Disk->getBlockAddr(data.dirBlock);
	strcpy(path, data.path);
	showAll();
	ui.textEdit->setText(QString(path));
}

//点击删除选中的item
void OSDesignProj3::deleteItem()
{
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();
	for (int i = 0; i < items.size(); i += 3)
	{
		myDataTable data = items[i]->data(Qt::UserRole).value<myDataTable>();
		if (data.type == 1)
		{//如果是文件，直接删除
			deleteFile(data.fileName);
			showAll(); //刷新
		}
		else
		{//如果是目录，从tree里也需要删除

			changeDir(data.fileName);//进入该目录，找到目标地址和路径
			QTreeWidgetItem* tItem = findItem(currentDirTable, path);
			char back[] = "..";
			changeDir(back); //返回上一级
			removeItem(tItem);
			deleteDir(data.fileName); //从磁盘删除
			showAll(); //刷新
		}
	}
}

//按路径搜索并跳转
void OSDesignProj3::search_path()
{
	QString text = ui.textEdit->toPlainText();
	std::string str = text.toStdString();
	const char* ch = str.data();
	//遍历treeWidget
	QTreeWidgetItemIterator it(ui.treeWidget);
	while (*it) {
		myData data = (*it)->data(0, Qt::UserRole + 1).value<myData>(); //遍历树的item属性
		if (strcmp(data.path, ch) == 0)
		{
			break;
		}
		++it;
	}
	//如果找到对应的item
	if (*it)
	{
		checkself(*it, 0);
	}
	else
	{
		QMessageBox::about(this, "注意", "路径错误！");
	}
}

/*--------------------------------------------------------------------------------*/

void removeItem(QTreeWidgetItem* item)
{
	int count = item->childCount();
	if (count == 0)//没有子节点，直接删除
	{
		delete item;
		return;
	}

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* childItem = item->child(0);//删除子节点
		removeItem(childItem);
	}
	delete item;//最后将自己删除
}

NoFocusDelegate::NoFocusDelegate()
{

}

NoFocusDelegate::~NoFocusDelegate()
{

}

void NoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem itemOption(option);
	// 去掉选中时的虚线框
	if (itemOption.state & QStyle::State_HasFocus)
	{
		itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	}
	// 设置选中的item，字体颜色和选中前的颜色一样
	itemOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::ForegroundRole).value<QColor>());
	QStyledItemDelegate::paint(painter, itemOption, index);
}
