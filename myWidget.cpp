#include "myWidget.h"

void myWidget::closeEvent(QCloseEvent* event/*, bool& save*/)
{//save为true则没有越界，反之则越界
	//bool enough = save; //是否越界
	QMessageBox msg;
	msg.setText("是否保存？");
	msg.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	if (msg.exec() == QMessageBox::No)
	{
		//save = false;
		return;
	}
	//if (enough)
	//{
	//	//save = true;
	//	return;
	//}
	//else
	//{
	//	QMessageBox::about(NULL, "", "保存失败，文本过长");
	//}
}
