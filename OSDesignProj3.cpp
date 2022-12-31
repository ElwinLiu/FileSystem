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
	//	QStringLiteral("����") << QStringLiteral("����")
	//	<< QStringLiteral("�޸�����"));
	//ui.tableWidget->setModel(model_);
	ui.tableWidget->verticalHeader()->hide(); //����ͼ��
	QStringList strs = { "����", "����" }; //�б���
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->verticalHeader()->setDefaultSectionSize(15);
	ui.tableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(204,232,255) }");//�ı�ѡ�е���ɫ
	ui.tableWidget->setItemDelegate(new NoFocusDelegate()); //���õ���ı��������
	ui.tableWidget->setColumnCount(2); //��������
	ui.tableWidget->setHorizontalHeaderLabels(strs); //�����б���
	//ui.tableWidget->horizontalHeader()->setStretchLastSection(true); //��ռ��
	ui.tableWidget->setShowGrid(false); //��������
	ui.tableWidget->setSelectionBehavior(QTableWidget::SelectRows); //һ��ѡ��һ��
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //���ò��ɱ༭
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu); //�Ҽ�׼��
	//��ʼ��ϵͳ
	Disk = new diskOperator;
	//������Ŀ¼
	initRootDir();
	connect(ui.tableWidget, SIGNAL(customContextMenuRequested(QPoint))
		, this, SLOT(show_menu(QPoint)));//�Ҽ���ʾ�˵�
	connect(ui.actionCreate_user, SIGNAL(triggered()), this, SLOT(on_click_create()));
	connect(ui.tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
		this, SLOT(slot_doubleClicked(QTableWidgetItem*)));
	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(click_back()));
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(checkself(QTreeWidgetItem*, int)));//���Ŀ¼�ṹ�ĵ���¼����źŲۻ���
	connect(ui.pushButton_3, SIGNAL(clicked(bool)), this, SLOT(deleteItem()));
	connect(ui.pushButton_2, SIGNAL(clicked(bool)), this, SLOT(search_path()));
}

void OSDesignProj3::initRootDir()
{
	Disk = new diskOperator;
	//����һ���̿�ռ��rootDirTable
	int startBlock = Disk->getBlock(1);
	if (startBlock == -1)
		return;
	rootDirTable = (dirTable*)Disk->getBlockAddr(startBlock);
	rootDirTable->dirUnitAmount = 0;
	//��������Ϊ����Ŀ¼
	//addDirUnit(rootDirTable, "..", 0, startBlock);

	currentDirTable = rootDirTable;
	//��ʼ����ʼ����·��
	path[0] = '\\';
	path[1] = '\0';
}

void OSDesignProj3::showDir()
{
	int unitAmount = currentDirTable->dirUnitAmount;
	printf("total %d\n", unitAmount);
	printf("name\ttype\tsize\tFCB\tdataStartBlock\n");
	//�������б���
	for (int i = 0; i < unitAmount; i++)
	{
		//��ȡĿ¼��
		dirUnit unitTemp = currentDirTable->dirs[i];
		printf("%s\t%d\t", unitTemp.fileName, unitTemp.type);
		//�ñ������ļ������������С����ʼ�̿��
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
	//ΪĿ¼�����ռ�
	int dirBlock = Disk->getBlock(1);
	if (dirBlock == -1)
		return -1;
	//��Ŀ¼��ΪĿ¼����ӵ���ǰĿ¼
	if (addDirUnit(currentDirTable, dirName, 0, dirBlock) == -1)
		return -1;
	//Ϊ�½���Ŀ¼���һ������Ŀ¼��Ŀ¼��
	dirTable* newTable = (dirTable*)Disk->getBlockAddr(dirBlock);
	newTable->dirUnitAmount = 0;
	char parent[] = "..";
	if (addDirUnit(newTable, parent, 0, Disk->getAddrBlock((char*)currentDirTable)) == -1)
		return -1;
	return 0;
}

int OSDesignProj3::changeDir(char dirName[])
{
	//Ŀ¼����Ŀ¼λ��
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	//������
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
	//�޸ĵ�ǰĿ¼
	int dirBlock = currentDirTable->dirs[unitIndex].startBlock;
	currentDirTable = (dirTable*)Disk->getBlockAddr(dirBlock); // ��ǰ�̿�λ��
	//�޸�ȫ�־���·��
	if (strcmp(dirName, "..") == 0)
	{
		//���˾���·��
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
	//����ϵͳ���Զ������ĸ�Ŀ¼
	if (strcmp(dirName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//�����ļ�
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == 0) //Ŀ¼
	{
		deleteFileInTable(currentDirTable, unitIndex);
	}
	else
	{
		printf("not a dir\n");
		return -1;
	}
	//��Ŀ¼�����޳�
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
	//����ļ����ֳ���
	if (strlen(fileName) >= 59)
	{
		printf("file name too long\n");
		return -1;
	}
	//���FCB�Ŀռ�
	int FCBBlock = Disk->getBlock(1);
	if (FCBBlock == -1)
		return -1;
	//��ȡ�ļ����ݿռ�
	int FileBlock = Disk->getBlock(fileSize);
	if (FileBlock == -1)
		return -1;
	//����FCB
	if (creatFCB(FCBBlock, FileBlock, fileSize) == -1)
		return -1;
	//��ӵ�Ŀ¼��
	if (addDirUnit(currentDirTable, fileName, 1, FCBBlock) == -1)
		return -1;

	return 0;
}

int OSDesignProj3::deleteFile(char fileName[])
{
	//����ϵͳ���Զ������ĸ�Ŀ¼
	if (strcmp(fileName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//�����ļ���Ŀ¼��λ��
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == 0) //Ŀ¼
	{
		printf("not a file\n");
		return -1;
	}
	int FCBBlock = myUnit.startBlock;
	//�ͷ��ڴ�
	releaseFile(FCBBlock);
	//��Ŀ¼�����޳�
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
	//���ƿ�
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	return reread(fileName, myFCB->dataSize); //��ȡ��������
}

QString OSDesignProj3::reread(char fileName[], int length)
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file no found\n");
		return -1;
	}
	//���ƿ�
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	myFCB->readptr = 0;
	return doRead(myFCB, length);
}

QString OSDesignProj3::doRead(FCB* myFCB, int length)
{
	QString store = "";//�洢����
	//������
	int dataSize = myFCB->dataSize;
	char* data = (char*)Disk->getBlockAddr(myFCB->blockNum);
	//�ڲ��������ݳ����£���ȡָ�����ȵ�����
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
	//���ƿ�
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
	//���ƿ�
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)Disk->getBlockAddr(FCBBlock);
	//�������ݿ�
	myFCB->dataSize = 0;
	myFCB->readptr = 0;

	return doWrite(myFCB, content);

}

bool OSDesignProj3::doWrite(FCB* myFCB, char content[])
{
	int contentLen = strlen(content);
	int fileSize = myFCB->fileSize * block_szie;
	char* data = (char*)Disk->getBlockAddr(myFCB->blockNum);
	//�ڲ������ļ��Ĵ�С�ķ�Χ��д��
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
	myFCB->link--; //��������һ
	//�����ӣ�ɾ���ļ�
	if (myFCB->link == 0)
	{
		//�ͷ��ļ������ݿռ�
		Disk->releaseBlock(myFCB->blockNum, myFCB->fileSize);
	}
	//�ͷ�FCB�Ŀռ�
	Disk->releaseBlock(FCBBlock, 1);
	return 0;
}

int OSDesignProj3::addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBBlockNum)
{
	//���Ŀ¼��
	int dirUnitAmount = myDirTable->dirUnitAmount;
	//���Ŀ¼��ʾ�Ƿ�����
	if (dirUnitAmount == dirTable_max_size)
	{
		printf("dirTables is full, try to delete some file\n");
		return -1;
	}

	//�Ƿ����ͬ���ļ�
	if (findUnitInTable(myDirTable, fileName) != -1)
	{
		printf("mkdir: %s: File exists\n", fileName);
		return -1;
	}
	//������Ŀ¼��
	dirUnit* newDirUnit = &myDirTable->dirs[dirUnitAmount];
	myDirTable->dirUnitAmount++; //��ǰĿ¼���Ŀ¼������+1
	//������Ŀ¼������
	strcpy(newDirUnit->fileName, fileName);
	newDirUnit->type = type;
	newDirUnit->startBlock = FCBBlockNum;

	return 0;
}

int OSDesignProj3::creatFCB(int fcbBlockNum, int fileBlockNum, int fileSize)
{
	//�ҵ�fcb�Ĵ洢λ��
	FCB* currentFCB = (FCB*)Disk->getBlockAddr(fcbBlockNum);
	currentFCB->blockNum = fileBlockNum; //�ļ�������ʼλ��
	currentFCB->fileSize = fileSize;     //�ļ���С
	currentFCB->link = 1;                //�ļ�������
	currentFCB->dataSize = 0;            //�ļ���д�����ݳ���
	currentFCB->readptr = 0;             //�ļ���ָ��
	return 0;
}

int OSDesignProj3::deleteFileInTable(dirTable* myDirTable, int unitIndex)
{
	//�����ļ�
	dirUnit myUnit = myDirTable->dirs[unitIndex];
	//�ж�����
	if (myUnit.type == 0) //Ŀ¼
	{
		//�ҵ�Ŀ¼λ��
		int FCBBlock = myUnit.startBlock;
		dirTable* table = (dirTable*)Disk->getBlockAddr(FCBBlock);
		//�ݹ�ɾ��Ŀ¼�µ������ļ�
		printf("cycle delete dir %s\n", myUnit.fileName);
		int unitCount = table->dirUnitAmount;
		for (int i = 1; i < unitCount; i++) //Ϊʲô���������ԡ�..��
		{
			printf("delete %s\n", table->dirs[i].fileName);
			deleteFileInTable(table, i);
		}
		//�ͷ�Ŀ¼��ռ�
		Disk->releaseBlock(FCBBlock, 1);
	}
	else
	{ //�ļ�
		//�ͷ��ļ��ڴ�
		int FCBBlock = myUnit.startBlock;
		releaseFile(FCBBlock);
	}
	return 0;
}

int OSDesignProj3::deleteDirUnit(dirTable* myDirTable, int unitIndex)
{
	//Ǩ�Ƹ���
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
	//���Ŀ¼��
	int dirUnitAmount = myDirTable->dirUnitAmount;
	int unitIndex = -1;
	for (int i = 0; i < dirUnitAmount; i++) //����Ŀ¼��λ��
		if (strcmp(unitName, myDirTable->dirs[i].fileName) == 0)
			unitIndex = i;
	return unitIndex;
}

//��TableWidget����ʾ�ü�Ŀ¼���еı���
void OSDesignProj3::showAll()
{
	ui.tableWidget->clearContents();//��ձ�
	ui.tableWidget->setRowCount(0);	int unitAmount = currentDirTable->dirUnitAmount;
	//�������б���
	for (int i = 0; i < unitAmount; i++)
	{
		if (strcmp(currentDirTable->dirs[i].fileName, "..") == 0) continue;
		//��ȡĿ¼��
		char* name = new char;
		strcpy(name, currentDirTable->dirs[i].fileName);
		QTableWidgetItem* btItem = new QTableWidgetItem();   //ͼ�� + �ļ��� 
		btItem->setTextAlignment(Qt::AlignCenter);			//���־���
		QString t;
		if (currentDirTable->dirs[i].type == 0)
		{//ΪĿ¼����
			btItem->setIcon(QIcon(":/OSDesignProj3/folder.png"));
			t = "�ļ���";
		}
		else
		{//Ϊ�ļ�����
			QTreeWidgetItem* item = findItem(currentDirTable, path);
			btItem->setIcon(QIcon(":/OSDesignProj3/file.png"));
			t = "�ļ�";
		}
		strcpy(name, currentDirTable->dirs[i].fileName);
		myDataTable dataTable(currentDirTable->dirs[i].type, name);
		btItem->setData(Qt::UserRole, QVariant::fromValue(dataTable));
		btItem->setText(currentDirTable->dirs[i].fileName);
		int RowCont; //��ǰҪд�ڼ���
		RowCont = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(RowCont);//����һ��
		ui.tableWidget->setItem(RowCont, 0, btItem);
		ui.tableWidget->setItem(RowCont, 1, new QTableWidgetItem(t));
	}
}

//����ļ�/�ļ���
void OSDesignProj3::insert(int type, char fileName[])
{
	QTableWidgetItem* btItem = new QTableWidgetItem();   //ͼ�� + �ļ��� 
	btItem->setTextAlignment(Qt::AlignCenter);			//���־���
	QString t;
	if (type == 0)
	{//ΪĿ¼����
		btItem->setIcon(QIcon(":/OSDesignProj3/folder.png"));
		t = "�ļ���";
		/*����ӵ����ļ��� ����ô��Ŀ¼�ṹ��Ҫ��ʾ���ļ��У��ڱ���ҲҪ��ʾ��*/
		creatDir(fileName);
		QTreeWidgetItem* item = findItem(currentDirTable, path);
		if (currentDirTable->dirUnitAmount >= 15) return;
		QString name = fileName;
		QTreeWidgetItem* new_item = new QTreeWidgetItem(item, QStringList(name));
		char* new_path = new char[200];
		//��ı�ָ��ָ��ǰĿ¼�Լ��ı�
		changeDir(fileName);
		strcpy(new_path, path);
		myData data(Disk->getAddrBlock((char*)currentDirTable), new_path);

		//���ظ�Ŀ¼
		char temp[] = "..";
		changeDir(temp);
		new_item->setData(0, Qt::UserRole + 1, QVariant::fromValue(data));
		//
		int test = 1;
		//
	}
	else
	{//Ϊ�ļ�����
		QTreeWidgetItem* item = findItem(currentDirTable, path);
		if (currentDirTable->dirUnitAmount >= 15) return;
		btItem->setIcon(QIcon(":/OSDesignProj3/file.png"));
		t = "�ļ�";
		creatFile(fileName, 100);
	}
	//��¼��������
	char* name = new char;
	strcpy(name, fileName);
	myDataTable dataTable(type, name);
	btItem->setData(Qt::UserRole, QVariant::fromValue(dataTable));
	btItem->setText(fileName);
	int RowCont; //��ǰҪд�ڼ���
	RowCont = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(RowCont);//����һ��
	ui.tableWidget->setItem(RowCont, 0, btItem);
	ui.tableWidget->setItem(RowCont, 1, new QTableWidgetItem(t));
}

QTreeWidgetItem* OSDesignProj3::findItem(dirTable* table, char* path)
{
	//����treeWidget
	QTreeWidgetItemIterator it(ui.treeWidget);
	while (*it) {
		myData data = (*it)->data(0, Qt::UserRole + 1).value<myData>(); //��������item����
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
	QString text = QInputDialog::getText(this, tr("�����û�"), tr("�������û���"), QLineEdit::Normal, 0, &ok);
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
		{//�����˻�ȥ
			currentDirTable = temp;
			strcpy(path, temp_path);
			return;
		}
		changeDir(name); //�Զ������Ŀ¼
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
	//���ò˵�ѡ��
	QMenu* menu = new QMenu(ui.tableWidget);
	QAction* pnew = new QAction("����ļ�", ui.tableWidget);
	QAction* pnew1 = new QAction("���Ŀ¼", ui.tableWidget);
	connect(pnew, SIGNAL(triggered()), this, SLOT(click_0()));
	connect(pnew1, SIGNAL(triggered()), this, SLOT(click_1()));
	menu->addAction(pnew);
	menu->addAction(pnew1);
	menu->move(cursor().pos());
	menu->show();
	////����������x��y�����
	//int x = pos().x();
	//int y = pos().y();
	//QModelIndex index = ui.tableWidget->indexAt(QPoint(x, y));
	//int row = index.row();//���QTableWidget�б���������
}

//����ļ�
void OSDesignProj3::click_0()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("�����ļ�"), tr("�������ļ���"), QLineEdit::Normal, 0, &ok);
	if (ok && !text.isEmpty())
	{
		std::string str = text.toStdString();
		const char* ch = str.data();
		char name[59] = {};
		strcpy(name, ch);
		insert(1, name);
	}
}

//���Ŀ¼
void OSDesignProj3::click_1()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("�����ļ���"), tr("�������ļ�����"), QLineEdit::Normal, 0, &ok);
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
	{//�����Ŀ¼
		ui.tableWidget->clearContents();
		ui.tableWidget->setRowCount(0);
		changeDir(data.fileName); //�����Ŀ¼
		int unitAmount = currentDirTable->dirUnitAmount;
		showAll();
		ui.textEdit->setText(QString(path));
	}
	else
	{//���˫���ļ�
		QMessageBox msg;
		msg.setText("������Ϣ��");
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		auto res = msg.exec();

		if (res == QMessageBox::Yes)
		{//�����ı���Ϣ
			bool ok;
			QString text = QInputDialog::getText(this, tr("�����ı���Ϣ"), tr("������"), QLineEdit::Normal, 0, &ok);
			if (ok && !text.isEmpty())
			{
				char* ch = new char;
				QByteArray ba = text.toLocal8Bit();
				ch = ba.data();
				if (write(data.fileName, ch)) return;
				else QMessageBox::about(this, "ע��", "���ȹ���������������ʡ��");
			}
		}
		else if (res == QMessageBox::No)
		{
			//�½����ڶ��� 
			QString text = read(data.fileName);
			QWidget* qw = new QWidget();
			qw->resize(800, 800);
			//�½�QTextEdit���󣬸�����Ϊqw
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
	myData data = item->data(0, Qt::UserRole + 1).value<myData>(); //��������item����
	currentDirTable = (dirTable*)Disk->getBlockAddr(data.dirBlock);
	strcpy(path, data.path);
	showAll();
	ui.textEdit->setText(QString(path));
}

//���ɾ��ѡ�е�item
void OSDesignProj3::deleteItem()
{
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems();
	for (int i = 0; i < items.size(); i += 3)
	{
		myDataTable data = items[i]->data(Qt::UserRole).value<myDataTable>();
		if (data.type == 1)
		{//������ļ���ֱ��ɾ��
			deleteFile(data.fileName);
			showAll(); //ˢ��
		}
		else
		{//�����Ŀ¼����tree��Ҳ��Ҫɾ��

			changeDir(data.fileName);//�����Ŀ¼���ҵ�Ŀ���ַ��·��
			QTreeWidgetItem* tItem = findItem(currentDirTable, path);
			char back[] = "..";
			changeDir(back); //������һ��
			removeItem(tItem);
			deleteDir(data.fileName); //�Ӵ���ɾ��
			showAll(); //ˢ��
		}
	}
}

//��·����������ת
void OSDesignProj3::search_path()
{
	QString text = ui.textEdit->toPlainText();
	std::string str = text.toStdString();
	const char* ch = str.data();
	//����treeWidget
	QTreeWidgetItemIterator it(ui.treeWidget);
	while (*it) {
		myData data = (*it)->data(0, Qt::UserRole + 1).value<myData>(); //��������item����
		if (strcmp(data.path, ch) == 0)
		{
			break;
		}
		++it;
	}
	//����ҵ���Ӧ��item
	if (*it)
	{
		checkself(*it, 0);
	}
	else
	{
		QMessageBox::about(this, "ע��", "·������");
	}
}

/*--------------------------------------------------------------------------------*/

void removeItem(QTreeWidgetItem* item)
{
	int count = item->childCount();
	if (count == 0)//û���ӽڵ㣬ֱ��ɾ��
	{
		delete item;
		return;
	}

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* childItem = item->child(0);//ɾ���ӽڵ�
		removeItem(childItem);
	}
	delete item;//����Լ�ɾ��
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
	// ȥ��ѡ��ʱ�����߿�
	if (itemOption.state & QStyle::State_HasFocus)
	{
		itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	}
	// ����ѡ�е�item��������ɫ��ѡ��ǰ����ɫһ��
	itemOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::ForegroundRole).value<QColor>());
	QStyledItemDelegate::paint(painter, itemOption, index);
}
