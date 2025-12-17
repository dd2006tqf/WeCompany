#-------------------------------------------------
#
# WeCompany Server Project
# TCP Server with Audio/Video Call Support
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = WeCompanyServer
CONFIG += console c++11
CONFIG -= app_bundle
TEMPLATE = app
DESTDIR = bin

INCLUDEPATH += server/include

SOURCES += server/source/servermain.cpp \
    server/source/tcpserver.cpp \
    server/source/videocallserver.cpp

HEADERS += server/include/tcpserver.h \
    server/include/videocallserver.h

# Default rules for deployment
unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
