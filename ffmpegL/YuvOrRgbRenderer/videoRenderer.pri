

!contains(DEFINES,VIDEORENDER){
DEFINES += VIDEORENDER
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



# 拷贝运行所需动态库去运行目录
#win32: {
#DLL_PATH = $$PWD/../bin/x86/*.dll
#DLL_PATH ~= s,/,\\,g
#DESTDIR_WIN = $$OUT_PWD
#DESTDIR_WIN ~= s,/,\\,g
#QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"$$DLL_PATH\"  \"$$DESTDIR_WIN\"
#message($$QMAKE_POST_LINK)
#}
SOURCES += \
    $$PWD/SDLRerderer.cpp \
    $$PWD/VideoRerdererView.cpp

HEADERS += \
    $$PWD/SDLRerderer.h \
    $$PWD/VideoRerdererView.h

INCLUDEPATH += $$PWD



include(../Logger/Logger.pri)

}
