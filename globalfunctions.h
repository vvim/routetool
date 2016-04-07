#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

/**
  Header file added because some functions have to be "called from several "global".
  Based on http://stackoverflow.com/questions/6874346/how-do-you-define-a-global-function-in-c/6874391

  In the first place, needed to be able to reconnect to the database whenever we please.
  In future, we could add a function to 'switch' database everytime the Configuration-widget is edited


  [COMMENT:] Maybe we should put these funtions in 'configuration.h'?
   -> NO, 'configuration.h' is NOT a header with global configuration code,
          it contains a class to read/edit the configuration-file. It has a specific purpose.
**/


#include "configuration.h"
#include <QDebug>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDateTime>

// [0] define global variables: configuration-file 'settings' and logfile 'debuglogfile'

extern QSettings settings;

extern FILE *debuglogfile;

// [1] global qDebug-tweak

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"



void checkSettings();

void myMessageOutput(QtMsgType type, const char *msg);

bool startLoggingToFile();

bool connectToDatabase();

bool reConnectToDatabase(QSqlError lasterror, QString SQLquery, QString callingfunction);

QString seconds_human_readable(int totalseconds);

QMap<int, QString> getQMapFromSQLTable(QString select_query);

#endif // GLOBALFUNCTIONS_H
