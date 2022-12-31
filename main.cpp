#include "OSDesignProj3.h"
#include <QtWidgets/QApplication>
#include <qtextcodec.h>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication a(argc, argv);
    OSDesignProj3 w;
    w.show();
    return a.exec();
}
