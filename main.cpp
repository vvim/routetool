#include <QtGui/QApplication>
#include <QSqlDatabase>
#include "routetool.h"
#include "configuration.h"
#include "mainwindow.h"

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

FILE *debuglogfile;

QSettings settings("routetool.ini", QSettings::IniFormat);

void checkSettings()
{
    // check if everything is filled in:
    QStringList needed_keys;
    needed_keys << "apiKey" <<  "db/databasename" << "db/host" << "db/password" << "db/username" << "startpunt" << "zak_kaarsresten_naar_kg" << "zak_kaarsresten_volume" << "zak_kurk_naar_kg" << "zak_kurk_volume" << "max_gewicht_vrachtwagen" << "max_volume_vrachtwagen";

    bool all_keys_found = true;

    foreach(QString key, needed_keys)
    {
        if(!settings.contains(key))
            all_keys_found = false;
    }

    if(!all_keys_found)
    {
        qDebug() << "show Dialog";
        Configuration *c = new Configuration();
        c->show();
    }

    qDebug() << "Configuration checkSettings(): OK! All settings are filled in";
}

void myMessageOutput(QtMsgType type, const char *msg)
{
    //in this function, you can write the message to any stream!
    switch (type) {
    case QtDebugMsg:
        fprintf(debuglogfile, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(debuglogfile, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(debuglogfile, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(debuglogfile, "Fatal: %s\n", msg);
        abort();
    }
}

bool startLoggingToFile()
{
    QDateTime currentdatetime = QDateTime::currentDateTime();
    QString filename = QString("routetool-%1.log").arg(currentdatetime.toString("yyyyMMddhhmmsszzz"));
    debuglogfile = fopen(filename.toStdString().c_str(), "a");
    if (NULL == debuglogfile) {
        QString error = "";
        error.append("cannot open debuglogfile '%1'").arg(filename);
        perror(error.toStdString().c_str());
        return false;
    }
    qInstallMsgHandler(myMessageOutput);
    return true;
}

bool connectToDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(settings.value("db/host").toString());
    db.setDatabaseName(settings.value("db/databasename").toString());
    db.setUserName(settings.value("db/username").toString() );
    db.setPassword(settings.value("db/password").toString());

    if( !db.open() )
    {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This is the error recieved: \n\n").append(db.lastError().text()), QMessageBox::Cancel);
        qCritical( "Failed to connect to database" );
        return false;
    }

    qDebug( "Connected to database" );

    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    checkSettings();

//    if (!startLoggingToFile()) exit(0);
    if (!connectToDatabase()) exit(0);

    RouteTool r;
    r.show();

    a.exec();

    qDebug() << "remove DB";
    QSqlDatabase::removeDatabase(settings.value("db/databasename").toString());


    return 0;
}


/**

Als samenvatting van ons telefoontje:

-optimaliseringstool met 20 adressen: zo snel mogelijk 'debugd', graag ook met databank
 ophaalpunten geïncorporeerd (tegen 2 juli)

-tegen 2 juli: invoeren van meldingen van op te halen hoeveelheden:
          * extra veld voor 'datum melding'. Na de ophaling moeten de effectief opgehaalde
                  hoeveelheden natuurlijk ook terug ingevoerd worden (in gelijkaardige, maar
                  andere velden + extra velden voor 'kilogrammen')

-planning: loopt uit, graag wat meer prioriteit voor dit project. De opleveringsdatum van alle
 werkzaamheden zou ten laatste 1 september moeten zijn (tenzij i.g.v. overmacht). In je nieuwe
 planning ontbreken nog de exportfuncties (tenzij dat onder iets anders vervat zit)? Misschien op
 2 juli toch wat vollediger proberen op te lijsten.

**/

/**
    * ophaalpunt dat zich heeft aangemled (kan je uit een keuzelijst kiezen)
    * datum melding
    * naam contactpersoon
    * # zakken kurk    | echte # zakken  (aparte knop "melding ingeven" , "ophaalronde bevestigen"
    * # kg kurk        | echte # kg
    * # zakken kaars   | echte # zakken
    * # kg kaars       | echte # kg
    * eventueel opmerkingen
**/
