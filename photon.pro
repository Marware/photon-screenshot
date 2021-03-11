#-------------------------------------------------
#
# Project created by QtCreator 2014-02-20T20:23:17
#
#-------------------------------------------------

QT       += core widgets network printsupport multimedia

TARGET = photon
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    localsocketipc.cpp

HEADERS  += mainwindow.h \
    localsocketipc.h

FORMS    += mainwindow.ui \
    progress.ui \
    ocredtext.ui \
    about.ui \
    settings.ui

RC_ICONS = logo.ico

RESOURCES += \
    rs.qrc

OTHER_FILES += \
    res.rc \
    logo.ico

RC_FILE = res.rc
