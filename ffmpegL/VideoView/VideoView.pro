QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Add.cpp \
    VVConfig.cpp \
    VVTools.cpp \
    VideoRecord.cpp \
    VideoWidget.cpp \
    main.cpp \
    VideoView.cpp

HEADERS += \
    Add.h \
    VVConfig.h \
    VVTools.h \
    VideoRecord.h \
    VideoView.h \
    VideoWidget.h

FORMS += \
    Add.ui \
    VideoView.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#设置运行目录
Video_RUN_TREE = $$PWD/../bin/x86
CONFIG(debug,debug|release) {
    DESTDIR = $${Video_RUN_TREE}/debug
} else {
    DESTDIR = $${Video_RUN_TREE}/release
}

#添加VideoDll库
win32:CONFIG(release, debug|release): LIBS += -L$${Video_RUN_TREE}/release/ -lVideoDll
else:win32:CONFIG(debug, debug|release): LIBS += -L$${Video_RUN_TREE}/debug/ -lVideoDll
else:unix: LIBS += -L$$OUT_PWD/../VideoDll/ -lVideoDll

INCLUDEPATH += $$PWD/../VideoDll
DEPENDPATH += $$PWD/../VideoDll


include(../Logger/Logger.pri)

RESOURCES += \
    res.qrc \
    res.qrc

RC_ICONS = ./image/app.ico
