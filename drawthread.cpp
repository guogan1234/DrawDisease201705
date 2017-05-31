#include "drawthread.h"
#include <QFile>
#include <QDebug>
#include <QImage>
#include "info.h"

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/opencv.hpp"

#include <QTime>
#include <iostream>
using namespace cv;

//线性、区域病害合并
#include <string>
#include <vector>
#include <stack>
#include <opencv2/opencv.hpp>

using namespace std;

extern QMap<int,sessionDisease> sessionMap;
extern QMap<int,sessionDisease>::iterator sessionMapIter;

extern QHash<int,diseaseType> typeHash;
extern QHash<int,diseaseType>::iterator typeHashIter;

//线性、区域病害合并
Mat mergeContours(Mat img,int ret);  // 合并区域病害
Mat mergeLines(Mat img,int ret);     // 合并线条病害

void getSkeleton(Mat &srcimage);  // 获取骨架图,输入为单通道、二值化后的图像
void getEight(Mat img, Mat &eight, vector<Point> &points); // 探讨八联通区域
void connectNearest(Mat &eight, vector<Point> &points, int ret, vector<int> conLabels); // 连接最近的两个点
double getEuclidean(Point A, Point B); // 欧氏距离
vector<int> getPair(vector<Point> points, vector<vector<Point>> contours);  // 轮廓和点配对
string num2str(int i);

DrawThread::DrawThread(QObject *parent) :
    QThread(parent)
{
}

DrawThread::DrawThread(int i, QObject *parent)
{
    m_count = i;

    contoursDis = 100;
    linesDis = 60;
}

DrawThread::~DrawThread()
{
    qDebug()<<"~DrawThread";
}

void DrawThread::run()
{
    bool b = CopyImages(m_count);//生成需要绘制的段图像
    if(b){
        CutLastImg(m_lastImgStart,m_lastImgEnd);
        CutLastImg2(m_lastImgStart,m_lastImgEnd);
        DrawDisease();

        //线性、区域病害合并
        qDebug()<<"1:contoursMat...";
        for(int i = 0;i<1;i++){
//        char* path1 = NULL;
//        char* path2 = NULL;
        Mat contoursMat;
        string str1 = "";
        Mat img1 = imread("./Data/0.jpg");
        qDebug()<<"2:contoursMat...";
        contoursMat = mergeContours(img1,contoursDis);// 合并区域病害
        imwrite("./0-dest.jpg",contoursMat);
        Mat linesMat;
        string str2 = "";
        Mat img2 = imread("./0.jpg");
        linesMat = mergeLines(img2,linesDis);// 合并线条病害
        imwrite("./0-dest.jpg",linesMat);
        }
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
    for(int a = 0;a < i;a++){
        QString destName = "./" + QString::number(a*50) + ".jpg";
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

void DrawThread::CutLastImg2(int start, int end)
{
    char ch_src[128];
    char* ch = "./";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
    Mat destImg = img.colRange(start,end);
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
//        sprintf(ch_src,"%s%d%s",ch,a*50,ext);

//        char* ch_src = "./Data/50.jpg";//opencv支持传入相对路径读取和操作图像
        qDebug()<<"ch_src:"<<ch_src;

        QList<QPointF> srcPts = sessionMapIter.value().pts;
        int len = srcPts.length();



        //根据病害类型设置画笔颜色
        CvScalar scalar;
        scalar.val[0] = 0;
        scalar.val[1] = 0;
        scalar.val[2] = 255;
        QString type = sessionMapIter.value().diseaseType;
        QString flag = "";
        char* ch_flag = "";
        flag = findDiseaseType_SX(type);
        if(flag == "lf"){//线性病害
            scalar.val[0] = 0;
            scalar.val[1] = 0;
            scalar.val[2] = 255;
            ch_flag = "lf";

            ch = "./";
        }
        else{//区域性病害
            ch = "./Data/";
            if(flag == "SGF_lf"){
                scalar.val[0] = 255;
                scalar.val[1] = 0;
                scalar.val[2] = 255;
                ch_flag = "SGF_lf";
            }
            else if(flag == "E"){
                scalar.val[0] = 0;
                scalar.val[1] = 255;
                scalar.val[2] = 0;
                ch_flag = "E";
            }
            else if(flag == "Wa"){
                scalar.val[0] = 255;
                scalar.val[1] = 0;
                scalar.val[2] = 0;
                ch_flag = "Wa";
            }
            else if(flag == "L_Po"){
                scalar.val[0] = 255;
                scalar.val[1] = 255;
                scalar.val[2] = 0;
                ch_flag = "L_Po";
            }
            else if(flag == "N_Po"){
                scalar.val[0] = 0;
                scalar.val[1] = 255;
                scalar.val[2] = 255;
                ch_flag = "N_Po";
            }
        }
        CvScalar textScalar;
        textScalar.val[0] = 100;
        textScalar.val[1] = 100;
        textScalar.val[2] = 100;

        CvFont font;
        cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX,3.0f,3.0f,1.0f,3);

        //加载图片
        sprintf(ch_src,"%s%d%s",ch,a*50,ext);
        IplImage* src_imageRGB = NULL;
        src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

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
                if(i == len - 2){//最后一个点上绘制病害类型
//                    char* ch_type = NULL;
//                    ch_type = flag.toLocal8Bit().data();
//                    qDebug()<<"cvPutText...";
//                    cvPutText(src_imageRGB,ch_flag,p2,&font,textScalar);
                }
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

Mat mergeContours(Mat img, int ret)
{
    Mat gray, thImg;
    if (img.channels()>1) cvtColor(img, gray, COLOR_BGR2GRAY);
    else img.copyTo(gray);
    threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);

    vector<vector<Point>> contours;
    Mat draw = Mat(thImg.size(), CV_8U, Scalar(0));
    findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    for (int i = 0; i < contours.size(); i++)
    {
        drawContours(draw, contours, i, Scalar(255), -1, 8);
    }
    //imshow("draw",draw);
    cout << "Before Merge : contours.size() = " << contours.size() << endl;

    Mat closeImg;
    morphologyEx(draw, closeImg, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(ret, ret)));
    //imshow("closeImg", closeImg);

    Mat draw2 = Mat(thImg.size(), CV_8UC3, Scalar(255, 255, 255));
    findContours(closeImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    for (int i = 0; i < contours.size(); i++)
    {
        drawContours(draw2, contours, i, Scalar(255, 0, 0), 5, 8);
    }
    cout << "After Merge :contours.size() = " << contours.size() << endl;
    return draw2;
}

Mat mergeLines(Mat sick, int ret)
{
    Mat gray, thImg, eight, dilateImg,draw,red;

    if (sick.channels()>1) cvtColor(sick, gray, COLOR_BGR2GRAY);
    else sick.copyTo(gray);
    threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);

    getSkeleton(thImg);

    thImg.copyTo(eight);
    vector<Point> points;

    getEight(thImg, eight, points);

    vector<vector<Point>> contours;
    vector<int> conLabels;
    findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    conLabels = getPair(points, contours);

    connectNearest(eight, points, ret, conLabels);

    morphologyEx(eight, dilateImg, MORPH_DILATE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    draw = Mat(dilateImg.size(), CV_8UC3, Scalar(255, 255, 255));
    red = Mat(dilateImg.size(), CV_8UC3, Scalar(0, 0, 255));
    red.copyTo(draw,dilateImg);
    return draw;
}

string num2str(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

vector<int> getPair(vector<Point> points, vector<vector<Point>> contours)
{
    double flag;
    vector<int> conLabels = vector<int>(points.size());
    for (int i = 0; i < points.size(); i++)
    {
        for (int j = 0; j < contours.size(); j++)
        {
            flag = pointPolygonTest(contours[j], points[i], false);
            if (0 == flag)
            {
                conLabels[i] = j;
            }
        }
    }
    return conLabels;
}


double getEuclidean(Point A, Point B)
{
    return sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
}

void connectNearest(Mat &eight, vector<Point> &points, int ret, vector<int> conLabels)
{
    double tempD;
    vector<double> dists;
    for (int i = 0; i < points.size(); i++)
    {
        int flag = 1;  // flag控制设置最初最小值的语句在每个次循环只运行一次
        int minV_idx = -1;  // 记录每个点满足要求的最小值时的点的索引
        double minValue;
        for (int j = i; j < points.size(); j++)
        {
            if (i == j) continue;
            tempD = getEuclidean(points[i], points[j]);
            if (conLabels[i] != conLabels[j] && tempD<ret) // 处在不同轮廓才记录
            {
                if (flag == 1)
                { // 只运行一次
                    minValue = tempD;
                    minV_idx = j;
                    flag == 0;
                    continue;
                }
                if (tempD < minValue)
                {
                    minValue = tempD;
                    minV_idx = j;
                }
            }
        }
        if (-1 != minV_idx) line(eight, points[i], points[minV_idx], Scalar(255), 1);
    }
}

void getEight(Mat img, Mat &eight, vector<Point> &points)
{
    int dataSum = 0;
    for (int i = 1; i < img.rows - 1; i++)
    {
        uchar* data_last = img.ptr<uchar>(i - 1);
        uchar* data = img.ptr<uchar>(i);
        uchar* data_next = img.ptr<uchar>(i + 1);
        for (int j = 1; j < (img.cols - 1); j++)
        {
            if (data[j] == 0) continue;
            dataSum = data[j - 1] + data[j + 1] + data_last[j] + data_last[j - 1] + data_last[j + 1] + data_next[j] + data_next[j - 1] + data_next[j + 1];
            if (dataSum == 255)
            {
                //circle(eight, Point(j, i), 3, Scalar(255)); // 是否标记出线的端点
                points.push_back(Point(j, i));
            }
        }
    }
}

void getSkeleton(Mat &srcimage)
{
    vector<Point> deletelist1;
    int Zhangmude[9];
    int nl = srcimage.rows;
    int nc = srcimage.cols;
    while (true)
    {
        for (int j = 1; j<(nl - 1); j++)
        {
            uchar* data_last = srcimage.ptr<uchar>(j - 1);
            uchar* data = srcimage.ptr<uchar>(j);
            uchar* data_next = srcimage.ptr<uchar>(j + 1);
            for (int i = 1; i<(nc - 1); i++)
            {
                if (data[i] == 255)
                {
                    Zhangmude[0] = 1;
                    if (data_last[i] == 255) Zhangmude[1] = 1;
                    else  Zhangmude[1] = 0;
                    if (data_last[i + 1] == 255) Zhangmude[2] = 1;
                    else  Zhangmude[2] = 0;
                    if (data[i + 1] == 255) Zhangmude[3] = 1;
                    else  Zhangmude[3] = 0;
                    if (data_next[i + 1] == 255) Zhangmude[4] = 1;
                    else  Zhangmude[4] = 0;
                    if (data_next[i] == 255) Zhangmude[5] = 1;
                    else  Zhangmude[5] = 0;
                    if (data_next[i - 1] == 255) Zhangmude[6] = 1;
                    else  Zhangmude[6] = 0;
                    if (data[i - 1] == 255) Zhangmude[7] = 1;
                    else  Zhangmude[7] = 0;
                    if (data_last[i - 1] == 255) Zhangmude[8] = 1;
                    else  Zhangmude[8] = 0;
                    int whitepointtotal = 0;
                    for (int k = 1; k < 9; k++)
                    {
                        whitepointtotal = whitepointtotal + Zhangmude[k];
                    }
                    if ((whitepointtotal >= 2) && (whitepointtotal <= 6))
                    {
                        int ap = 0;
                        if ((Zhangmude[1] == 0) && (Zhangmude[2] == 1)) ap++;
                        if ((Zhangmude[2] == 0) && (Zhangmude[3] == 1)) ap++;
                        if ((Zhangmude[3] == 0) && (Zhangmude[4] == 1)) ap++;
                        if ((Zhangmude[4] == 0) && (Zhangmude[5] == 1)) ap++;
                        if ((Zhangmude[5] == 0) && (Zhangmude[6] == 1)) ap++;
                        if ((Zhangmude[6] == 0) && (Zhangmude[7] == 1)) ap++;
                        if ((Zhangmude[7] == 0) && (Zhangmude[8] == 1)) ap++;
                        if ((Zhangmude[8] == 0) && (Zhangmude[1] == 1)) ap++;
                        if (ap == 1)
                        {
                            if ((Zhangmude[1] * Zhangmude[7] * Zhangmude[5] == 0) && (Zhangmude[3] * Zhangmude[5] * Zhangmude[7] == 0))
                            {
                                deletelist1.push_back(Point(i, j));
                            }
                        }
                    }
                }
            }
        }
        if (deletelist1.size() == 0) break;
        for (size_t i = 0; i < deletelist1.size(); i++)
        {
            Point tem;
            tem = deletelist1[i];
            uchar* data = srcimage.ptr<uchar>(tem.y);
            data[tem.x] = 0;
        }
        deletelist1.clear();

        for (int j = 1; j<(nl - 1); j++)
        {
            uchar* data_last = srcimage.ptr<uchar>(j - 1);
            uchar* data = srcimage.ptr<uchar>(j);
            uchar* data_next = srcimage.ptr<uchar>(j + 1);
            for (int i = 1; i<(nc - 1); i++)
            {
                if (data[i] == 255)
                {
                    Zhangmude[0] = 1;
                    if (data_last[i] == 255) Zhangmude[1] = 1;
                    else  Zhangmude[1] = 0;
                    if (data_last[i + 1] == 255) Zhangmude[2] = 1;
                    else  Zhangmude[2] = 0;
                    if (data[i + 1] == 255) Zhangmude[3] = 1;
                    else  Zhangmude[3] = 0;
                    if (data_next[i + 1] == 255) Zhangmude[4] = 1;
                    else  Zhangmude[4] = 0;
                    if (data_next[i] == 255) Zhangmude[5] = 1;
                    else  Zhangmude[5] = 0;
                    if (data_next[i - 1] == 255) Zhangmude[6] = 1;
                    else  Zhangmude[6] = 0;
                    if (data[i - 1] == 255) Zhangmude[7] = 1;
                    else  Zhangmude[7] = 0;
                    if (data_last[i - 1] == 255) Zhangmude[8] = 1;
                    else  Zhangmude[8] = 0;
                    int whitepointtotal = 0;
                    for (int k = 1; k < 9; k++)
                    {
                        whitepointtotal = whitepointtotal + Zhangmude[k];
                    }
                    if ((whitepointtotal >= 2) && (whitepointtotal <= 6))
                    {
                        int ap = 0;
                        if ((Zhangmude[1] == 0) && (Zhangmude[2] == 1)) ap++;
                        if ((Zhangmude[2] == 0) && (Zhangmude[3] == 1)) ap++;
                        if ((Zhangmude[3] == 0) && (Zhangmude[4] == 1)) ap++;
                        if ((Zhangmude[4] == 0) && (Zhangmude[5] == 1)) ap++;
                        if ((Zhangmude[5] == 0) && (Zhangmude[6] == 1)) ap++;
                        if ((Zhangmude[6] == 0) && (Zhangmude[7] == 1)) ap++;
                        if ((Zhangmude[7] == 0) && (Zhangmude[8] == 1)) ap++;
                        if ((Zhangmude[8] == 0) && (Zhangmude[1] == 1)) ap++;
                        if (ap == 1)
                        {
                            if ((Zhangmude[1] * Zhangmude[3] * Zhangmude[5] == 0) && (Zhangmude[3] * Zhangmude[1] * Zhangmude[7] == 0))
                            {
                                deletelist1.push_back(Point(i, j));
                            }
                        }
                    }
                }
            }
        }
        if (deletelist1.size() == 0) break;
        for (size_t i = 0; i < deletelist1.size(); i++)
        {
            Point tem;
            tem = deletelist1[i];
            uchar* data = srcimage.ptr<uchar>(tem.y);
            data[tem.x] = 0;
        }
        deletelist1.clear();
    }
}

QString DrawThread::findDiseaseType_SX(QString type)
{
    QString temp = "";
    for(typeHashIter = typeHash.begin();typeHashIter != typeHash.end();typeHashIter++){
        if(typeHashIter.value().type == type){
            temp = typeHashIter.value().type_SX;
        }
//        qDebug()<<"findDiseaseType_SX:"<<typeHashIter.value().id<<temp;
    }
    return temp;
}
