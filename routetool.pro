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
#voeg mysql.lib toe
}

win32-msvc* {
    LIBS += ../qjson/build/lib/qjson0.lib
#voeg mysql.lib toe
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
    documentwriter.cpp \
    configuration.cpp \
    nieuweaanmelding.cpp \
    infoophaalpunt.cpp \
    mainwindow.cpp \
    kiesophaalpunten.cpp

HEADERS  += routetool.h \
    form.h \
    geocode_data_manager.h \
    distancematrix.h \
    smarker.h \
    mylineedit.h \
    documentwriter.h \
    configuration.h \
    nieuweaanmelding.h \
    infoophaalpunt.h \
    mainwindow.h \
    kiesophaalpunten.h

FORMS    += routetool.ui \
    form.ui



RESOURCES += \
    resource.qrc

