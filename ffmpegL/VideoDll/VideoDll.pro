QT -= gui

TEMPLATE = lib
DEFINES += VIDEODLL_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CTools.cpp \
    Codec.cpp \
    Decodec.cpp \
    DecodecThread.cpp \
    Demux.cpp \
    DemuxThread.cpp \
    Encodec.cpp \
    Format.cpp \
    Muxing.cpp \
    MuxingThread.cpp \
    SDLRerderer.cpp \
    VideoRerdererView.cpp

HEADERS += \
    CTools.h \
    Codec.h \
    Decodec.h \
    DecodecThread.h \
    Demux.h \
    DemuxThread.h \
    Encodec.h \
    Format.h \
    Muxing.h \
    MuxingThread.h \
    SDLRerderer.h \
    VideoDll_global.h \
    VideoRerdererView.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

!contains(DEFINES,VIDEO_LIB){
DEFINES += VIDEO_LIB
win32: LIBS += -L$$PWD/../lib/x86/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lSDL2 \
                 -lswresample -lswscale

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

#设置运行目录
Video_RUN_TREE = $$PWD/../bin/x86
CONFIG(debug,debug|release) {
    DESTDIR = $${Video_RUN_TREE}/debug
} else {
    DESTDIR = $${Video_RUN_TREE}/release
}


include(../Logger/Logger.pri)
}
