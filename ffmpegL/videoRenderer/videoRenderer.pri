

!contains(DEFINES,VIDEORENDER){
DEFINES += VIDEORENDER
INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32: {
LIBS += -L$$PWD/../lib/x86/  -lSDL2
LIBS += -L$$PWD/../bin/x86/  -lSDL2
DLL_PATH = $$PWD/../bin/x86/SDL2.dll
DLL_PATH ~= s,/,\\,g
DESTDIR_WIN = $$OUT_PWD
DESTDIR_WIN ~= s,/,\\,g
QMAKE_POST_LINK += $$escape_expand(\\n) $$QMAKE_COPY \"$$DLL_PATH\"  \"$$DESTDIR_WIN\"
message($$QMAKE_POST_LINK)
}

SOURCES += \
    $$PWD/SDLRerderer.cpp \
    $$PWD/VideoRerdererView.cpp \

HEADERS += \
    $$PWD/SDLRerderer.h \
    $$PWD/VideoRerdererView.h

INCLUDEPATH += $$PWD

include(../Logger/Logger.pri)

}
