#include "configuration.h"
#include "globalfunctions.h"
#include <QDebug>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <math.h>


// [0] define global variables: configuration-file 'settings' and logfile 'debuglogfile'

QSettings settings("lat-lng-b.ini", QSettings::IniFormat);

FILE *debuglogfile;

void checkSettings()
{
    /**
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
        vvimDebug() << QObject::tr("show Dialog");
        Configuration *c = new Configuration();
        c->show();

        // how to correctly delete "c" after it is done? // why not make it c.show() ???
    }

    vvimDebug() << QObject::tr("Configuration checkSettings(): OK! All settings are filled in.");
    **/
}

void myMessageOutput(QtMsgType type, const char *msg)
{
    //in this function, you can write the message to any stream!
    switch (type) {
    case QtDebugMsg:
        fprintf(debuglogfile, "%s Debug: %s\n", QDateTime::currentDateTime().toString("[dd/MM/yy hh:mm:ss:zzz]").toStdString().c_str(), msg);
        break;
    case QtWarningMsg:
        fprintf(debuglogfile, "%s Warning: %s\n", QDateTime::currentDateTime().toString("[dd/MM/yy hh:mm:ss:zzz]").toStdString().c_str(), msg);
        break;
    case QtCriticalMsg:
        fprintf(debuglogfile, "%s Critical: %s\n", QDateTime::currentDateTime().toString("[dd/MM/yy hh:mm:ss:zzz]").toStdString().c_str(), msg);
        break;
    case QtFatalMsg:
        fprintf(debuglogfile, "%s Fatal: %s\n", QDateTime::currentDateTime().toString("[dd/MM/yy hh:mm:ss:zzz]").toStdString().c_str(), msg);
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
        error.append(QObject::tr("cannot open debuglogfile '%1'")).arg(filename);
        perror(error.toStdString().c_str());
        return false;
    }
    qInstallMsgHandler(myMessageOutput);
    return true;
}


// [2] function to connect to the database

bool connectToDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(settings.value("db/host").toString());
    db.setDatabaseName(settings.value("db/databasename").toString());
    db.setUserName(settings.value("db/username").toString() );
    db.setPassword(settings.value("db/password").toString());
    qDebug() << QObject::tr(" ++ 1) original connect options:") << db.connectOptions();
    db.setConnectOptions( "MYSQL_OPT_RECONNECT=true;" ) ;
    qDebug() << QObject::tr(" ++ 2) new connect options:") << db.connectOptions();

    if( !db.open() )
    {
        /*
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This is the error recieved: \n\n").append(db.lastError().text()), QMessageBox::Cancel);
                     */
        QMessageBox::critical(0, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                     "This is the error recieved: \n\n").append(db.lastError().text()), QMessageBox::Cancel);
        qCritical( "Failed to connect to database" );
        return false;
    }

    qDebug() << " ++ 3) connect options after opening:" << db.connectOptions();
    vvimDebug() << "Connected to database at " << settings.value("db/host").toString();

    return true;
}

bool reConnectToDatabase(QSqlError lasterror, QString SQLquery, QString callingfunction)
{
    vvimDebug() << callingfunction << QObject::tr("Cannot execute query. DB lost? Try to reconnect!") << lasterror.text();

    if(!connectToDatabase())
    {
        QString textforlogfile = QString(QObject::tr("Something went wrong, could not reconnect to execute query: %1 error %2").arg(SQLquery).arg(lasterror.text()));
        vvimDebug() << callingfunction << "FATAL:" << textforlogfile ;
        qFatal((textforlogfile).toStdString().c_str());
        return false;
    }
    else
    {
        vvimDebug() << callingfunction << QObject::tr("Reconnection successful!");
        return true;
    }
}


QString seconds_human_readable(int totalseconds)
{
    QString human_readable = "";
    int hours = floor(totalseconds/3600.0);
    int minutes = floor(fmod(totalseconds,3600.0)/60.0);
    int seconds = fmod(totalseconds,60.0);

    /* // check for calculation error:
    int secondscheck = (((((0*24)+hours)*60) + minutes)*60) + seconds;
    if (secondscheck == totalseconds)
        vvimDebug() << "OK";
    else
        vvimDebug() << "[error]";
    */

    human_readable.sprintf("%01du %02dm %02ds", hours, minutes, seconds);
    return human_readable;
}

QMap<int, QString> getQMapFromSQLTable(QString select_query)
{

    vvimDebug() << ".. we zouden ook van elke soort een aparte functie kunnen maken, die wordt aangeroepen bij opstart, kan je dan meteen ook gebruiken voor de DialogBox van InformatieOphaalpunt, zonder dat je opnieuw de databank moet aanroepen";

    QSqlQuery query;
    query.prepare(select_query);
    query.exec();
    QMap <int,QString> result;
    while (query.next())
    {
        result[query.value(0).toInt()] = query.value(1).toString();
    }

    /*
      for debugging:

    QMap<int, QString>::iterator it;
    QString output;

    for (it = contact_preference.begin(); it != contact_preference.end(); ++it) {
        // Format output here.
        output += QString("...... %1 : %2\n").arg(it.key()).arg(it.value());
    }
    vvimDebug() << output;
    */

    return result;
}

QString JavaScriptEscape(QString plaintext)
{
    QString javascriptversion = plaintext.replace("\n"," ").replace("'","\\'");
    javascriptversion.replace("\\\\'","\\'");
    return javascriptversion;
}
