#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T12:08:12
#
#-------------------------------------------------

QT       += core gui webkit network
QT           += sql

TARGET = routetool
TEMPLATE = app

INCLUDEPATH += ../qjson/include

linux* {
INCLUDEPATH += /usr/include/cppconn

LIBS += ../qjson/build/lib/libqjson.so
LIBS += /usr/lib/libmysqlclient.so
LIBS += /usr/lib/libmysqlcppconn.so
#LIBS += /usr/lib/qt4/plugins/sqldrivers/libqsqlmysql.so
#LIBS += /home/wim/QtSDK/QtSources/4.7.3/src/plugins/sqldrivers/mysql-build-desktop-Desktop_Qt_4_7_3_for_GCC__Qt_SDK__Release/libqsqlmysql.so
LIBS += /home/wim/QtSDK/QtSources/4.7.3/src/plugins/sqldrivers/mysql/libqsqlmysql.so
}

win32-g++ {
    LIBS += ../qjson/build/lib/qjson0.dll
    LIBS += ../qjson/build/lib/libqjson.dll
    LIBS += C:\\Wim\\MySQL\\MySQLServer55\\lib\\libmysql.dll
    LIBS += C:\\Wim\\MySQL\\MySQLServer55\\lib\\libmysql.lib
}

win32-msvc* {
    LIBS += ../qjson/build/lib/qjson0.lib
    LIBS += ../qjson/build/lib/libqjson.dll
    LIBS += C:\\Wim\\MySQL\\MySQLServer55\\lib\\libmysql.dll
    LIBS += C:\\Wim\\MySQL\\MySQLServer55\\lib\\libmysql.lib
}

macx* {
LIBS += -F../qjson/build/lib -framework qjson
}


SOURCES += main.cpp\
        routetool.cpp \
    form.cpp \
    geocode_data_manager.cpp \
    distancematrix.cpp \
    mylineedit.cpp \
    configuration.cpp \
    nieuweaanmelding.cpp \
    infoophaalpunt.cpp \
    kiesophaalpunten.cpp \
    levering.cpp \
    listofophaalpuntentocontact.cpp \
    transportationlistwriter.cpp \
    transportationlistdocumentwriter.cpp \
    ophaalpuntenwidget.cpp \
    mysortfilterproxymodel.cpp \
    ophaalhistoriekdialog.cpp \
    kiesgedaneophaling.cpp \
    opgehaaldehoeveelheid.cpp \
    exportcollectionhistory.cpp \
    ophaalhistoriekdialogsortfilterproxymodel.cpp \
    listofophaalpuntentocontactsortfilterproxymodel.cpp \
    globalfunctions.cpp \
    ophaalpuntenwidgetsortfilterproxymodel.cpp

HEADERS  += routetool.h \
    form.h \
    geocode_data_manager.h \
    distancematrix.h \
    smarker.h \
    mylineedit.h \
    configuration.h \
    nieuweaanmelding.h \
    infoophaalpunt.h \
    kiesophaalpunten.h \
    sophaalpunt.h \
    levering.h \
    slevering.h \
    listofophaalpuntentocontact.h \
    transportationlistwriter.h \
    transportationlistdocumentwriter.h \
    ophaalpuntenwidget.h \
    mysortfilterproxymodel.h \
    ophaalhistoriekdialog.h \
    kiesgedaneophaling.h \
    opgehaaldehoeveelheid.h \
    exportcollectionhistory.h \
    ophaalhistoriekdialogsortfilterproxymodel.h \
    listofophaalpuntentocontactsortfilterproxymodel.h \
    globalfunctions.h \
    ophaalpuntenwidgetsortfilterproxymodel.h

FORMS    += routetool.ui \
    form.ui \
    ophaalhistoriekdialog.ui

