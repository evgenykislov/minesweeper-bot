#-------------------------------------------------
#
# Project created by QtCreator 2019-03-09T23:41:44
#
#-------------------------------------------------

QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = minesweeper_bot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    botdialog.cpp \
    screen.cpp \
    celltypedialog.cpp \
    classifier.cpp \
    tetragonal_heural.cpp

HEADERS += \
    botdialog.h \
    screen.h \
    celltypedialog.h \
    classifier.h \
    tetragonal_neural.h

FORMS += \
    botdialog.ui \
    celltypedialog.ui

RESOURCES +=

unix:!macx: LIBS += -L/usr/local/lib/ -luiohook -lfakeInput

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include
