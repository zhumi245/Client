QT       += core network
CONFIG   += c++11 console
CONFIG   -= app_bundle
CONFIG   -= qt_gui

TARGET    = final_simulator
TEMPLATE  = app

SOURCES  += \
    main.cpp \
    simulator.cpp

HEADERS  += \
    simulator.h

INCLUDEPATH += .
