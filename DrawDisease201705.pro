#-------------------------------------------------
#
# Project created by QtCreator 2017-05-19T16:46:13
#
#-------------------------------------------------

QT       += core gui axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DrawDisease201705
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    drawthread.cpp

HEADERS  += widget.h \
    info.h \
    drawthread.h

FORMS    += widget.ui

INCLUDEPATH += ./opencv244/opencv/include

#F:\\QtMyProjectTest\\DrawDisease201705\\opencv244\\opencv\\lib
LIBS += -lopencv_calib3d244.dll -lopencv_contrib244.dll -lopencv_core244.dll\
    -lopencv_features2d244.dll -lopencv_flann244.dll -lopencv_gpu244.dll -lopencv_highgui244.dll\
    -lopencv_imgproc244.dll -lopencv_legacy244.dll -lopencv_ml244.dll -lopencv_nonfree244.dll\
    -lopencv_objdetect244.dll -lopencv_photo244.dll -lopencv_stitching244.dll -lopencv_ts244\
    -lopencv_video244.dll -lopencv_videostab244.dll -lzbar.dll -L./opencv244/opencv/lib
