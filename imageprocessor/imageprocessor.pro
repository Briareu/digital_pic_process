QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    angle.cpp \
    input.cpp \
    lab3.cpp \
    lab4.cpp \
    main.cpp \
    mywidget.cpp \
    picform.cpp \
    show_img.cpp

HEADERS += \
    angle.h \
    input.h \
    lab3.h \
    lab4.h \
    mywidget.h \
    picType.h \
    picform.h \
    show_img.h

FORMS += \
    angle.ui \
    input.ui \
    lab3.ui \
    lab4.ui \
    mywidget.ui \
    picform.ui \
    show_img.ui

INCLUDEPATH += E:\opencv\opencv\opencv-build\install\include

LIBS += -L E:\opencv\opencv\opencv-build\install\x64\mingw\lib\libopencv_*.a


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
