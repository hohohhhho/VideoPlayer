QT       += core gui multimedia multimediawidgets network
# RC_ICONS += res/iocn.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# ggml.file = $$PWD/thirdparty/whisper/ggml/ggml.c
# ggml.CONFIG += c
# SOURCES += ggml

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    contextbutton.cpp \
    emotionanalysisworker.cpp \
    gifwidget.cpp \
    headbutton.cpp \
    main.cpp \
    mainwindow.cpp \
    musicwidget.cpp \
    mycombobox.cpp \
    myhintpushbutton.cpp \
    myvideowidget.cpp \
    networkwidget.cpp \
    progressbar.cpp \
    ringbuffer.cpp \
    setwidget.cpp \
    userlistwidget.cpp \
    videobutton.cpp \
    videowidget.cpp \
    widgetbox.cpp \
    # $$PWD/thirdparty/whisper/ggml/src/ggml.c \
    # $$PWD/thirdparty/whisper/src/whisper.cpp \
    # $$PWD/thirdparty/whisper/ggml/src/ggml-alloc.c \
    # $$PWD/thirdparty/whisper/ggml/src/ggml-quants.c

HEADERS += \
    contextbutton.h \
    emotionanalysisworker.h \
    gifwidget.h \
    headbutton.h \
    mainwindow.h \
    musicwidget.h \
    mycombobox.h \
    myhintpushbutton.h \
    myvideowidget.h \
    networkwidget.h \
    progressbar.h \
    ringbuffer.h \
    setwidget.h \
    userlistwidget.h \
    videobutton.h \
    videowidget.h \
    widgetbox.h \

FORMS += \
    mainwindow.ui \
    networkwidget.ui \
    setwidget.ui \
    userlistwidget.ui

# INCLUDEPATH += $$PWD/whisper.cpp/include
# INCLUDEPATH += $$PWD/whisper.cpp/ggml/include
# INCLUDEPATH += \
#             $$PWD/thirdparty/whisper/include \
#             $$PWD/thirdparty/whisper/src \
#             $$PWD/thirdparty/whisper/ggml \
#             $$PWD/thirdparty/whisper/ggml/src \
#             $$PWD/thirdparty/whisper/ggml/include

INCLUDEPATH += $$PWD/thirdparty
#                 D:\Qt2.0\ffmpeg\include

# LIBS += -L F:\Download\opencv-4.x\opencv-4.x\build\Desktop_Qt_6_8_0_MinGW_64_bit-Release\lib\libopencv_*.dll.a\
#         -L D:\Qt2.0\ffmpeg\lib\lib*.dll.a


# INCLUDEPATH += D:\Qt2.0\opencv\install\include

LIBS += -L $$PWD/thirdparty/lib/libopencv_*.dll.a

# LIBS += -L $$PWD/whisper.cpp/build/Desktop_Qt_6_8_0_MinGW_64_bit-Debug/src -lwhisper
# LIBS += -L $$PWD/whisper.cpp/build/Desktop_Qt_6_8_0_MinGW_64_bit-Debug/ggml/src -lggml

# LIBS += -L $$PWD/whisper.cpp/build/Desktop_Qt_6_8_0_MinGW_64_bit-Debug/examples -lcommon
# LIBS += -lwhisper

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
