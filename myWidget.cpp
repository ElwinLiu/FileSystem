#include "myWidget.h"

void myWidget::closeEvent(QCloseEvent* event/*, bool& save*/)
{//saveΪtrue��û��Խ�磬��֮��Խ��
	//bool enough = save; //�Ƿ�Խ��
	QMessageBox msg;
	msg.setText("�Ƿ񱣴棿");
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
	//	QMessageBox::about(NULL, "", "����ʧ�ܣ��ı�����");
	//}
}
