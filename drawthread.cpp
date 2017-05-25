#include "drawthread.h"
#include <QFile>
#include <QDebug>
#include <QImage>
#include "info.h"

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include <QTime>
#include <iostream>
using namespace cv;

extern QMap<int,sessionDisease> sessionMap;
extern QMap<int,sessionDisease>::iterator sessionMapIter;

DrawThread::DrawThread(QObject *parent) :
    QThread(parent)
{
}

DrawThread::DrawThread(int i, QObject *parent)
{
    m_count = i;
}

DrawThread::~DrawThread()
{
    qDebug()<<"~DrawThread";
}

void DrawThread::run()
{
    bool b = CopyImages(m_count);
    if(b){
        CutLastImg(m_lastImgStart,m_lastImgEnd);//调试中
        DrawDisease();
    }else{
        ;
    }
    qDebug()<<"End draw!!!";
}

bool DrawThread::CopyImages(int i)
{
    qDebug()<<"CopyImages:"<<i;
    emit sendRemain(-1,0);

    bool b = true;
    QFile img("./Data/muban.jpg");
    for(int a = 0;a < i;a++){
        QString destName = "./Data/" + QString::number(a*50) + ".jpg";
        bool temp = img.copy(destName);
        qDebug()<<"temp:"<<temp;
        if(!temp){
            b = temp;
            return b;
        }
    }
    emit sendRemain(-2,0);
    return b;
}

void DrawThread::CutLastImg(int start, int end)
{
    char ch_src[128];
    char* ch = "./Data/";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    qDebug()<<"1111";
//    char* chSrc = ch_src;
//    std::string strSrc = ch_src;
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
//    std::cout<<img.cols;
    qDebug()<<"###"<<img.cols;
    Mat destImg = img.colRange(start,end);
    qDebug()<<"222";
//    std::string destPath = "C://dest.jpg";
//    imwrite(destPath,destImg);
    cvSaveImage(ch_src,&IplImage(destImg));
}

void DrawThread::DrawDisease()
{
    qDebug()<<"DrawDisease...";
    int total = sessionMap.size();
    emit sendRemain(total,0);
    QTime tt;
    tt.start();
    for(sessionMapIter = sessionMap.begin();sessionMapIter != sessionMap.end();sessionMapIter++){
        int a = sessionMapIter.value().index;
//        QString str = "F:/QtMyProjectTest/DrawDisease201705/Data/" + QString::number(a*50) + ".jpg";
//        char* ch_src = str.toLatin1().data();

        char ch_src[128];
        char* ch = "./Data/";
        char* ext = ".jpg";
        sprintf(ch_src,"%s%d%s",ch,a*50,ext);

//        char* ch_src = "./Data/50.jpg";//opencv支持传入相对路径读取和操作图像
        qDebug()<<"ch_src:"<<ch_src;

        QList<QPointF> srcPts = sessionMapIter.value().pts;
        int len = srcPts.length();

        IplImage* src_imageRGB = NULL;
        src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);
        CvScalar scalar;
        scalar.val[0] = 0;
        scalar.val[1] = 0;
        scalar.val[2] = 255;
        if(len == 1){//只有1个点
            ;
        }else{
            for(int i = 0;i<len-1;i++){
                CvPoint p1;
                p1.x = srcPts[i].x();
                p1.y = srcPts[i].y();
                CvPoint p2;
                p2.x = srcPts[i+1].x();
                p2.y = srcPts[i+1].y();
                cvLine(src_imageRGB,p1,p2,scalar,5);
            }
        }
        cvSaveImage(ch_src,src_imageRGB);
        cvReleaseImage(&src_imageRGB);

        int ms = tt.elapsed();
        tt.restart();
        total--;
        emit sendRemain(total,ms);
    }
}
