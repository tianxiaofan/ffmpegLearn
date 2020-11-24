

!contains(DEFINES, LOGGER){
DEFINES += LOGGER
CONFIG += c++11

HEADERS += \
    $$PWD/Logger.h \
    $$PWD/LoggerTemplate.h \

SOURCES += \
    $$PWD/Logger.cpp

INCLUDEPATH += $$PWD
}

