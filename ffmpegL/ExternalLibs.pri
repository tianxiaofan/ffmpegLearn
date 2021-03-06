
!contains(DEFINES,FFMPEG){
DEFINES += FFMPEG
win32: LIBS += -L$$PWD/lib/x86/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lSDL2 \
                 -lswresample -lswscale

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32: {
DLL_PATH = $$PWD/bin/x86/*.dll
DLL_PATH ~= s,/,\\,g
DESTDIR_WIN = $$OUT_PWD
DESTDIR_WIN ~= s,/,\\,g
QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"$$DLL_PATH\"  \"$$DESTDIR_WIN\"
message($$QMAKE_POST_LINK)
}

}



