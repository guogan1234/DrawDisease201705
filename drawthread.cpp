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

//病害长度等信息(像素单位)
struct SickProperties
{
    Point site;
    double lenOrArea;
    int type;   // 1: 线 ,0: 区域
};

using namespace std;

extern QMap<int,sessionDisease> sessionMap;
extern QMap<int,sessionDisease>::iterator sessionMapIter;

extern QHash<int,diseaseType> typeHash;
extern QHash<int,diseaseType>::iterator typeHashIter;

//线性、区域病害合并
Mat mergeContours(Mat img,int ret);  // 合并区域病害
Mat mergeLines(Mat img,int ret);     // 合并线条病害

void getSkeleton(Mat &srcimage);  // 获取骨架图,输入为单通道、二值化后的图像
void getEight(Mat img, vector<Point> &points); // 探讨八联通区域
void connectNearest(Mat &eight, vector<Point> &points, int ret, vector<int> conLabels); // 连接最近的两个点
double getEuclidean(Point A, Point B); // 欧氏距离
vector<int> getPair(vector<Point> points, vector<vector<Point> > contours);  // 轮廓和点配对
string num2str(int i);

// 计算病害信息,true为线状病害,false区域病害
vector<SickProperties> computeSickProperties(Mat img,bool type);

// vector排序算子
bool algoComp(const Point &A, const Point &B);

bool finalOK;
QList<finalDiseaseInfo> finalDiseaseList;

//隧道上下行标识
extern bool isMinToMax;

DrawThread::DrawThread(QObject *parent) :
    QThread(parent)
{
}

DrawThread::DrawThread(int i, QObject *parent)
{
    m_count = i;

    InitParams();
}

DrawThread::~DrawThread()
{
    qDebug()<<"~DrawThread";
}

void DrawThread::tunnelNoToMile(QString no)
{
    QStringList list = no.split("+");
    int k = list[0].right(3).toInt();
    double m = list[1].toDouble();
    tunnelMile = k*1000.0 + m;
    qDebug()<<"tunnelNoToMile:"<<no<<tunnelMile;
}

QString DrawThread::tunnelMileToNo(double mile)
{
    QString temp;
    double totalM = mile/1000;//单位mm转为m
    int k = (mile/1000)/1000;
    double m = totalM - k*1000;
    temp = "K" + QString::number(k) + "+" + QString::number(m);
    qDebug()<<"tunnelMileToNo:"<<totalM<<k<<m<<temp<<mile;
    return temp;
}

void DrawThread::InitOnePixel()
{
    onePixelW = sessionMile/bigImgW;
    onePixelH = tunnelArc/bigImgH;
    onePixelArea = onePixelW*onePixelH;
}

void DrawThread::InitParams()
{
    contoursDis = 100;
    linesDis = 60;

    sessionMile = 50000;//50*1000mm
    tunnelArc = 20000;//单位mm,取20m=20*1000mm
    bigImgW = 11877;
    bigImgH = 4948;


    InitOnePixel();
    finalOK = false;
}

void DrawThread::run()
{
    qDebug()<<"run...";
    tunnelNoToMile(tunnelNo);//tunnelNo = "K145+380.0";
    bool b = CopyImages(m_count);//生成需要绘制的段图像
//    bool b = true;//测试使用
    if(b){
        CutLastImg(m_lastImgStart,m_lastImgEnd);
        CutLastImg2(m_lastImgStart,m_lastImgEnd);
        DrawDisease();

        //线性、区域等病害合并
        MergeDisease();
        //计算病害长度等信息
        CalculateDisease();
        ShowFinalDiseaseList();
        //合并线性、区域图片
        mergeImages();

        //处理完成
        finalOK = true;
        emit workFinished();
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
    Mat destImg = img.colRange(end-start,end);
    qDebug()<<"CutLastImg:"<<start<<end;
    //    std::string destPath = "C://dest.jpg";
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));//opencv版本1函数
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
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
    Mat destImg = img.colRange(end-start,end);
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
}

void DrawThread::CutLastImg3(int start, int end)
{
    char ch_src[128];
    char* ch = "./Data/E/";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
    Mat destImg = img.colRange(start,end);
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
}

void DrawThread::CutLastImg4(int start, int end)
{
    char ch_src[128];
    char* ch = "./Data/Wa/";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
    Mat destImg = img.colRange(start,end);
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
}

void DrawThread::CutLastImg5(int start, int end)
{
    char ch_src[128];
    char* ch = "./Data/L_Po/";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
    Mat destImg = img.colRange(start,end);
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
}

void DrawThread::CutLastImg6(int start, int end)
{
    char ch_src[128];
    char* ch = "./Data/N_Po/";
    char* ext = ".jpg";
    sprintf(ch_src,"%s%d%s",ch,m_count*50-50,ext);
    IplImage* src_imageRGB = NULL;
    src_imageRGB = cvLoadImage(ch_src,CV_LOAD_IMAGE_ANYCOLOR);

    Mat img(src_imageRGB);
    Mat destImg = img.colRange(start,end);
    imwrite(ch_src,destImg);
    //    cvSaveImage(ch_src,&IplImage(destImg));
    cvReleaseImage(&src_imageRGB);
    img.release();
    destImg.release();
}

void DrawThread::DrawDisease()
{
    qDebug()<<"DrawDisease...";
    int total = sessionMap.size();
    emit sendRemain(total,0);
    QTime tt;
    tt.start();
    qDebug()<<"sessionMap len:"<<sessionMap.size();
    for(sessionMapIter = sessionMap.begin();sessionMapIter != sessionMap.end();sessionMapIter++){
        int a = sessionMapIter.value().index;
        //        QString str = "F:/QtMyProjectTest/DrawDisease201705/Data/" + QString::number(a*50) + ".jpg";
        //        char* ch_src = str.toLatin1().data();

        char ch_src[128];
        char* ch = "./Data/";
        char* ext = ".jpg";
        //        sprintf(ch_src,"%s%d%s",ch,a*50,ext);

        //        char* ch_src = "./Data/50.jpg";//opencv支持传入相对路径读取和操作图像
//        qDebug()<<"ch_src:"<<ch_src;

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
//                ch = "./Data/SGF/";
                scalar.val[0] = 255;
                scalar.val[1] = 0;
                scalar.val[2] = 255;
                ch_flag = "SGF_lf";
            }
            else if(flag == "E"){
//                ch = "./Data/E/";
                scalar.val[0] = 0;
                scalar.val[1] = 255;
                scalar.val[2] = 0;
                ch_flag = "E";
            }
            else if(flag == "Wa"){
//                ch = "./Data/Wa/";
                scalar.val[0] = 255;
                scalar.val[1] = 0;
                scalar.val[2] = 0;
                ch_flag = "Wa";
            }
            else if(flag == "L_Po"){
//                ch = "./Data/L_Po/";
                scalar.val[0] = 255;
                scalar.val[1] = 255;
                scalar.val[2] = 0;
                ch_flag = "L_Po";
            }
            else if(flag == "N_Po"){
//                ch = "./Data/N_Po/";
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
        qDebug()<<"ch_src:"<<ch_src;
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
        srcPts.clear();

        int ms = tt.elapsed();
        tt.restart();
        total--;
        emit sendRemain(total,ms);
    }
}

void DrawThread::MergeDisease()
{
    qDebug()<<"1:contoursMat...";
    for(int i = 0;i<m_count;i++){
        //区域病害合并
        //SGF
        Mat contoursMat;
        string str1 = "";
        char ch_src[128];
        char* ch1 = "./Data/";
        char* ext = ".jpg";
        sprintf(ch_src,"%s%d%s",ch1,i*50,ext);
        Mat img1 = imread(ch_src);
        qDebug()<<"2:contoursMat...";
        contoursMat = mergeContours(img1,contoursDis);// 合并区域病害
        img1.release();
        char ch_srcDest1[128];
        char* chDest = "-dest";
        sprintf(ch_srcDest1,"%s%d%s%s",ch1,i*50,chDest,ext);
        imwrite(ch_srcDest1,contoursMat);
        contoursMat.release();       
        //线性病害合并
        Mat linesMat;
        string str2 = "";
        char ch_src2[128];
        char* ch2 = "./";
        sprintf(ch_src2,"%s%d%s",ch2,i*50,ext);
        Mat img2 = imread(ch_src2);
        linesMat = mergeLines(img2,linesDis);// 合并线条病害
        img2.release();
        char ch_srcDest2[128];
        sprintf(ch_srcDest2,"%s%d%s%s",ch2,i*50,chDest,ext);
        imwrite(ch_srcDest2,linesMat);
        linesMat.release();
    }
}

void DrawThread::CalculateDisease()
{
    //读取图像路径
    char ch_src[128];
    char* ch1 = "./";
    char* ch2 = "./Data/";
    char* ext = ".jpg";
    char* chDest = "-dest";
    //可封装，改善
    //线性病害计算
    for(int i = 0;i<m_count;i++){
        sprintf(ch_src,"%s%d%s%s",ch1,i*50,chDest,ext);
        Mat img = imread(ch_src);

        bool flag = true;
//        int count = 0;
        vector<SickProperties> sickMsg = computeSickProperties(img, flag);
        vector<SickProperties>::iterator it;
        for(it=sickMsg.begin();it!=sickMsg.end();it++)
        {
            cout<<"@@@:"<<(*it).lenOrArea<<" "<<(*it).type<<endl;
            double px = (*it).site.x;
            double py = (*it).site.y;
            double lenOrArea = (*it).lenOrArea;
            CalculatePixelToMile(i,px,py,lenOrArea,flag);
        }
//        sickMsg.clear();
        img.release();
    }
    //区域性病害计算-SGF
    for(int i = 0;i<m_count;i++){
        sprintf(ch_src,"%s%d%s%s",ch2,i*50,chDest,ext);
        Mat img = imread(ch_src);

        bool flag = false;
//        int count = 0;
        vector<SickProperties> sickMsg = computeSickProperties(img, flag);
        vector<SickProperties>::iterator it;
        for(it=sickMsg.begin();it!=sickMsg.end();it++)
        {           
            double px = (*it).site.x;
            double py = (*it).site.y;
            double lenOrArea = (*it).lenOrArea;
            qDebug()<<"@@@:"<<(*it).lenOrArea<<(*it).type<<px<<py<<endl;
            CalculatePixelToMile(i,px,py,lenOrArea,flag);
        }
//        sickMsg.clear();
        img.release();
    }
//    finalOK = true;
//    emit workFinished();
}

void DrawThread::CalculatePixelToMile(int count, double px, double py, double lenOrArea, bool flag)
{
    finalDiseaseInfo temp;

    double start = 0;
    if(isMinToMax){
        start = tunnelMile*1000 + count*sessionMile + px*onePixelW;//单位mm
    }else{
        start = tunnelMile*1000 - count*sessionMile - px*onePixelW;//单位mm
    }
    qDebug()<<"onePixel:"<<tunnelMile<<tunnelMile*1000<<onePixelW<<onePixelH<<start;
    temp.startMile = tunnelMileToNo(start);
    double disRight = py*onePixelH;
    if(disRight > tunnelArc/2){
        disRight = tunnelArc - disRight;
        temp.IsMin = false;
    }else{
        disRight = disRight;
        temp.IsMin = true;
    }
    temp.distanceRight = disRight/1000;
    if(flag){
        temp.length = (lenOrArea*onePixelW)/1000;
        temp.area = 0;
    }else{
        temp.area = (lenOrArea*onePixelArea)/1000/1000;
        temp.length = 0;
    }
    finalDiseaseList.append(temp);
}

void DrawThread::ShowFinalDiseaseList()
{
    for(int i = 0;i<finalDiseaseList.length();i++){
        qDebug()<<"@@@:"<<i<<finalDiseaseList[i].startMile<<finalDiseaseList[i].IsMin<<finalDiseaseList[i].distanceRight<<finalDiseaseList[i].length<<finalDiseaseList[i].area;
    }
}

void DrawThread::mergeImages()
{
    for(int i = 0;i < m_count;i++){
        //SGF和line
        Mat lineImg,region;
        char ch_src[128];
        char ch_src2[128];
        char* ch1 = "./Data/";
        char* ch2 = "./";
        char* chParam1 = "-dest";
        char* ext = ".jpg";
        sprintf(ch_src,"%s%d%s%s",ch1,i*50,chParam1,ext);
        sprintf(ch_src2,"%s%d%s%s",ch2,i*50,chParam1,ext);
        region = imread(ch_src);
        lineImg = imread(ch_src2);

        //合并最终病害图片
        Mat merge = lineImg&region;
        char chDest[128];
        char* chParam2 = "-final";
        sprintf(chDest,"%s%d%s%s",ch2,i*50,chParam2,ext);
        imwrite(chDest,merge);
        //释放图片资源
        lineImg.release();
        region.release();
        merge.release();
    }
}

Mat mergeContours(Mat img, int ret)
{
    Mat gray, thImg;
    if (img.channels()>1) cvtColor(img, gray, COLOR_BGR2GRAY);
    else img.copyTo(gray);
    img.release();

    threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);
    gray.release();

    vector<vector<Point> > contours;
    Mat draw = Mat(thImg.size(), CV_8U, Scalar(0));
    findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    for (int i = 0; i < contours.size(); i++)
    {
        drawContours(draw, contours, i, Scalar(255), -1, 8);
    }
    thImg.release();
    //imshow("draw",draw);
    cout << "Before Merge : contours.size() = " << contours.size() << endl;

    Mat closeImg;
    morphologyEx(draw, closeImg, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(ret, ret)));
    draw.release();

    //imshow("closeImg", closeImg);

    Mat draw2 = Mat(closeImg.size(), CV_8UC3, Scalar(255, 255, 255));
    findContours(closeImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    for (int i = 0; i < contours.size(); i++)
    {
        drawContours(draw2, contours, i, Scalar(255, 0, 0), 5, 8);
    }
    cout << "After Merge :contours.size() = " << contours.size() << endl;
    contours.clear();
    return draw2;
}

Mat mergeLines(Mat sick, int ret)
{
    cout<<"mergeLines..."<<endl;
    Mat gray, thImg, eight, dilateImg, draw, red;
    if (sick.channels()>1) cvtColor(sick, gray, COLOR_BGR2GRAY);
    else sick.copyTo(gray);
    sick.release();

    threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);
    gray.release();

    getSkeleton(thImg);
    thImg.copyTo(eight);
    vector<Point> points;
    getEight(thImg, points);
    thImg.release();

    vector< vector<Point> > contours;
    vector<int> conLabels;
    findContours(eight, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    conLabels = getPair(points, contours);
    connectNearest(eight, points, ret, conLabels);
    morphologyEx(eight, dilateImg, MORPH_DILATE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    eight.release();

    cout<<"mergeLines:111..."<<endl;
    draw = Mat(dilateImg.size(), CV_8UC3, Scalar(255, 255, 255));
    cout<<"mergeLines:222..."<<endl;
    red = Mat(dilateImg.size(), CV_8UC3, Scalar(0, 0, 255));
    red.copyTo(draw,dilateImg);
    red.release();
    dilateImg.release();

    points.clear();
    contours.clear();
    conLabels.clear();
    return draw;
}

string num2str(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

vector<int> getPair(vector<Point> points, vector<vector<Point> > contours)
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
        if (-1 != minV_idx)
        {
            vector<double> dists;
            int k1,k2;

            // line(eight, points[i], points[minV_idx], Scalar(255), 1);
            // 连线之后，conLabel应该改为一致,这样就不会连接第二条线了
            for (int k = 0; k < conLabels.size(); k++)
            {
                if (conLabels[k] == conLabels[minV_idx])  // 最多只会满足一次
                {
                    k1 = k;
                }
                if (conLabels[k] == conLabels[i])
                {
                    k2 = k;
                }
            }
            conLabels[k1] = conLabels[i];
            conLabels[minV_idx] = conLabels[i];

            dists.push_back(getEuclidean(points[i], points[minV_idx]));
            dists.push_back(getEuclidean(points[i], points[k1]));
            dists.push_back(getEuclidean(points[k2], points[minV_idx]));
            dists.push_back(getEuclidean(points[k2], points[k1]));

//            vector<double>::iterator smallest = min_element(find_first_of(dists), find_end(dists));
//            int ind = distance(find_first_of(dists), smallest);
            int ind = 0;
            double minD = dists[0];
            for(int i=1;i<dists.size();i++)
            {
                if(dists[i]<minD) ind = i;
            }
//            int ind = 1;
            switch (ind)
            {
            case 0: line(eight, points[i], points[minV_idx], Scalar(255), 1); break;
            case 1: line(eight, points[i], points[k1], Scalar(255), 1); break;
            case 2: line(eight, points[k2], points[minV_idx], Scalar(255), 1); break;
            case 3: line(eight, points[k2], points[k1], Scalar(255), 1); break;
            }
        }
    }
}

void getEight(Mat img, vector<Point> &points)
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

bool algoComp(const Point &A, const Point &B)
{
    if (A.x < B.x) return true;
    return false;
}

vector<SickProperties> computeSickProperties(Mat img, bool type)
{
    Mat gray, thImg, draw;
    vector<vector<Point> > contours;

    cvtColor(img, gray, COLOR_BGR2GRAY);
    threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);
    findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
    draw = Mat(thImg.size(), CV_8UC3, Scalar(0, 0, 0));

    for (int i = 0; i < contours.size(); i++)
    {
        drawContours(draw, contours, i, Scalar(255, 255, 255), 1, 8);
    }
//    namedWindow("draw", WINDOW_NORMAL);
//    imshow("draw", draw);

    vector<SickProperties> SickMsg;
    SickProperties sickProperty;
    double lenOrArea;
    for (int i = 0; i < contours.size(); i++)
    {
        type ? lenOrArea = arcLength(contours[i], true)/2 : lenOrArea = contourArea(contours[i]);

        sort(contours[i].begin(), contours[i].end(), algoComp);
        //circle(draw, contours[i][0],50,Scalar(0,0,255),10);
        sickProperty.site = contours[i][0]; // 起始坐标site
        sickProperty.lenOrArea = lenOrArea;
        sickProperty.type = type;

        SickMsg.push_back(sickProperty);
    }

//像素值高度转换，只取小于一半隧道的弧长(即高度)
//    for (int i = 0; i < SickMsg.size(); i++)
//    {
//        if (SickMsg[i].site.y > round(img.rows / 2))
//        {
//            SickMsg[i].site.y = img.rows - SickMsg[i].site.y;
//        }
//    }

    for (int i = 0; i < SickMsg.size(); i++)
    {
        cout << "-------------SickMsg["<<i<<"]--------------" << endl;
        cout << "type: "<< SickMsg[i].type << endl;
        cout << "site: "<< SickMsg[i].site << endl;
        cout << "lenOrArea: "<<SickMsg[i].lenOrArea << endl;
    }

//    namedWindow("draw result", WINDOW_NORMAL);
//    imshow("draw result", draw);
    gray.release();
    thImg.release();
    draw.release();

    return SickMsg;
}
