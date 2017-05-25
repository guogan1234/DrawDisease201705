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

INCLUDEPATH += ./OpencvLib/include

LIBS += -L./OpencvLib/vLibsAndDll -lopencv_core244 -lopencv_highgui244 -lopencv_calib3d244\
    -lopencv_contrib244 -lopencv_features2d244 -lopencv_flann244 -lopencv_gpu244 -lopencv_imgproc244 -lopencv_legacy244\
    -lopencv_ml244 -lopencv_nonfree244 -lopencv_objdetect244 -lopencv_photo244 -lopencv_stitching244 -lopencv_ts244\
    -lopencv_video244 -lopencv_videostab244
