#include <QApplication>
#include <QStringList>
#include "idatasource.h"
#include "mockdatasource.h"
#include "networkclient.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("CarDashboard"));

    IDataSource *dataSource = 0;
    QStringList args = app.arguments();

    if (args.contains(QString::fromLatin1("--connect"))) {
        dataSource = new NetworkClient();
    } else {
        dataSource = new MockDataSource();
    }

    MainWindow w(dataSource);
    w.show();

    int ret = app.exec();
    if (dataSource) {
        dataSource->stop();
    }
    return ret;
}