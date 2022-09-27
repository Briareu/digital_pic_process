QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mywidget.cpp \
    picform.cpp

HEADERS += \
    mywidget.h \
    picType.h \
    picform.h

FORMS += \
    mywidget.ui \
    picform.ui

INCLUDEPATH += E:\opencv\opencv\opencv-build\install\include

LIBS += -L E:\opencv\opencv\opencv-build\install\x64\mingw\lib\libopencv_*.a


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
