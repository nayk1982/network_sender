#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
//==============================================================================
int main(int argc, char *argv[])
{
    /* Единственной кодировкой должна быть UTF-8, а использование других следует приравнять
     * к разжиганию межнациональной розни и карать соответствующей статьёй УК. */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);

    a.setOrganizationName("LPM");
    a.setApplicationName("NetworkSender");

    MainWindow w;
    w.show();
    return a.exec();
}
//==============================================================================
