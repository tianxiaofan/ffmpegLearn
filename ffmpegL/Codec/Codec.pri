!contains(DEFINES,CODEC){
DEFINES += CODEC
SOURCES += \
    $$PWD/Codec.cpp \
    $$PWD/Decodec.cpp \
    $$PWD/Encodec.cpp

HEADERS += \
    $$PWD/Codec.h \
    $$PWD/Decodec.h \
    $$PWD/Encodec.h

INCLUDEPATH += $$PWD

#没有重新引入ffmpeg相关库,头文件,直接引用了YuvOrRgbRenderer/videoRenderer.pri,在最终项目中需要更改
include(../YuvOrRgbRenderer/videoRenderer.pri)
}
