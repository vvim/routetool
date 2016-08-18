#include <QtGui/QApplication>
#include <QSqlDatabase>
#include "routetool.h"
#include "configuration.h"
#include "globalfunctions.h"

#include <QDebug>
#include <QJson/Parser>
#include <QVariantList>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QSettings>
#include <QSqlError>
#include <QMessageBox>
#include <QTextCodec>



int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    debuglogfile = NULL;

    vvimDebug() << "<vvim> TODO: in databank table ophaalhistoriek kan Tourist Union het ophaalpuntid 0 hebben, terwijl in table ophaalpunten het id 131 heeft. Best ophaalhistoriek aanpassen!";
    QApplication a(argc, argv);
    QLocale curLocale(QLocale("nl_NL"));
    QLocale::setDefault(curLocale);

    checkSettings();

    vvimDebug() << "databank:" << settings.value("db/host").toString();
    //if (!startLoggingToFile()) exit(0);
    if (!connectToDatabase()) exit(0);

    RouteTool r;
    r.show();

    a.exec();

    vvimDebug() << "remove DB";
    QSqlDatabase::removeDatabase(settings.value("db/databasename").toString());



    /*
    if(debuglogfile)
        fclose(debuglogfile);
    if(debuglogfile)
        delete debuglogfile;


    how to correctly close the logfile? This gives errors:
    vvimDebug() << "close logfile";
    fclose(debuglogfile);
    vvimDebug() << "delete logfile";
    delete debuglogfile;
    */

    return 0;
}
