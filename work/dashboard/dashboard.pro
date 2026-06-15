QT       += core gui widgets network multimedia
CONFIG   += c++11
TARGET    = dashboard
TEMPLATE  = app

SOURCES  += \
    main.cpp \
    mockdatasource.cpp \
    networkclient.cpp \
    mainwindow.cpp \
    speedometer.cpp \
    rpmgauge.cpp \
    fuelgauge.cpp \
    tempgauge.cpp \
    turnindicator.cpp \
    warninglights.cpp \
    warningsoundsystem.cpp

HEADERS  += \
    idatasource.h \
    mockdatasource.h \
    networkclient.h \
    mainwindow.h \
    speedometer.h \
    rpmgauge.h \
    fuelgauge.h \
    tempgauge.h \
    turnindicator.h \
    warninglights.h \
    warningsoundsystem.h

INCLUDEPATH += .