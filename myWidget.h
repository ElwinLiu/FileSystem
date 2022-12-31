#pragma once
#include <iostream>
#include <QCloseEvent>
#include <qwidget.h>
#include <qmessagebox.h>
#include "diskOperate.h"
using namespace std;

class myWidget : public QWidget
{
protected:
	void closeEvent(QCloseEvent* event/*, bool& save*/);
public:
	myWidget() {};
	~myWidget() {};
};