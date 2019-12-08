#-------------------------------------------------
#
# Project created by QtCreator 2019-03-09T23:41:44
#
#-------------------------------------------------

QT       += core gui testlib widgets
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
    models/classifier.cpp \
    models/tetragonal_neural.cpp \
    lineeditwfocus.cpp \
    imagesstorage.cpp \
    field.cpp \
    settingsdialog.cpp \
    easylogging++.cc \
    models/bruteforce.cpp \
    aboutdialog.cpp \
    licensesdialog.cpp

HEADERS += \
    botdialog.h \
    screen.h \
    celltypedialog.h \
    models/classifier.h \
    models/tetragonal_neural.h \
    lineeditwfocus.h \
    imagesstorage.h \
    field.h \
    settingsdialog.h \
    common.h \
    easylogging++.h \
    models/bruteforce.h \
    aboutdialog.h \
    licensesdialog.h

FORMS += \
    botdialog.ui \
    celltypedialog.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    licensesdialog.ui

RESOURCES += \
    bot.qrc

unix:!macx: LIBS += -L/usr/local/lib/
win32: LIBS += -L$$PWD/../win32/lib/

LIBS += -luiohook -lfakeInput

unix:!macx: INCLUDEPATH += /usr/local/include
win32: INCLUDEPATH += $$PWD/../win32/include

INCLUDEPATH += $$PWD

unix:!macx: DEPENDPATH += /usr/local/include
win32: DEPENDPATH += $$PWD/../win32/include

win32: PRE_TARGETDEPS += $$PWD/../win32/lib/uiohook.lib
win32: PRE_TARGETDEPS += $$PWD/../win32/lib/fakeInput.lib

win32 {
  LIBS += -luser32
}
