#-------------------------------------------------
#
# WeCompany Server Project v2.0
# Enhanced with Authentication, Database, and Agora SDK
#
#-------------------------------------------------

QT       += core network sql
QT       -= gui

TARGET = WeCompanyServer
CONFIG += console c++11
CONFIG -= app_bundle
TEMPLATE = app
DESTDIR = bin

INCLUDEPATH += server/include

SOURCES += server/source/servermain.cpp \
    server/source/tcpserver.cpp \
    server/source/videocallserver.cpp \
    server/source/authmanager.cpp \
    server/source/databasemanager.cpp \
    server/source/agoramanager.cpp

HEADERS += server/include/tcpserver.h \
    server/include/videocallserver.h \
    server/include/authmanager.h \
    server/include/databasemanager.h \
    server/include/agoramanager.h

# MySQL driver check
!contains(QT_SQL_DRIVERS, mysql) {
    message("Warning: MySQL driver not found. Database features will be disabled.")
    message("Install libqt5sql5-mysql on Linux or configure Qt with MySQL support")
}

# Default rules for deployment
unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
