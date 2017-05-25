#include "widget.h"
#include "ui_widget.h"

#include "info.h"
#include <QFileDialog>
#include <QAxObject>
#include <QDebug>

#include "drawthread.h"

//测试添加
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <iostream>
using namespace cv;
using namespace std;

InOutInfo inOutData;
QList<diseaseInfo> diseaseList;
QMap<int,sessionDisease> sessionMap;
QMap<int,sessionDisease>::iterator sessionMapIter;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    InitParams();
    ui->label->setVisible(false);
    ui->testBtn->setVisible(false);
    ui->hideWidget->setVisible(false);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    diseaseList.clear();
    sessionMap.clear();
    ClearToRestart();
}

void Widget::on_readInOut_clicked()
{
    QString xls1_path = QFileDialog::getOpenFileName(this,"file","D:/","xls (*.xls);;xlsx (*.xlsx)");
    ui->inOutLE->setText(xls1_path);

    qDebug()<<"xls1_path:"<<xls1_path;
    if(xls1_path != ""){
        readExcel(xls1_path,1);
    }
}

void Widget::on_readDisease_clicked()
{
    QString xls2_path = QFileDialog::getOpenFileName(this,"file","D:/","xls (*.xls);;xlsx (*.xlsx)");
    ui->diseaseLE->setText(xls2_path);

    if(xls2_path != ""){
        readExcel(xls2_path,2);
    }
}

void Widget::readExcel(QString path, int flag)
{
    if(flag == 1){
        readInOutExcel(path);
    }
    else if(flag == 2){
        readDiseaseExcel(path);
    }
}

void Widget::readInOutExcel(QString path)
{
    char* excelPath = path.toLocal8Bit().data();
    QAxObject excel("Excel.Application");
    excel.setProperty("Visble",false);
    QAxObject* workbooks = excel.querySubObject("WorkBooks");
    QAxObject* workbook = workbooks->querySubObject("Open(QString,QVariant,QVariant)", path,3,true);
    QAxObject* worksheet1 = workbook->querySubObject("Worksheets(int)",1);

    QAxObject* usedRange = worksheet1->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    //Excel索引从1开始
    int intRowStart = usedRange->property("Row").toInt();
    int intColStart = usedRange->property("Column").toInt();
    int intRows = rows->property("Count").toInt();
    int intCols = columns->property("Count").toInt();
    qDebug()<<intRowStart<<intColStart<<intRows<<intCols;
    QAxObject * range1 = worksheet1->querySubObject("Cells(int,int)", 1, 1 );
    QVariant var1 = range1->property("Value");
    QAxObject * range2 = worksheet1->querySubObject("Cells(int,int)", 1, 2 );
    QVariant var2 = range2->property("Value");
    QAxObject * range3 = worksheet1->querySubObject("Cells(int,int)", 1, 3 );
    QVariant var3 = range3->property("Value");
    QAxObject * range4 = worksheet1->querySubObject("Cells(int,int)", 1, 4 );
    QVariant var4 = range4->property("Value");

    inOutData.name = var1.toString();
    QString str2 = var2.toString();
    inOutData.inImgIndex = getIndexByImgName(str2);
    QString str3 = var3.toString();
    inOutData.outImgIndex = getIndexByImgName(str3);
    inOutData.dis = var4.toDouble();
    qDebug()<<"@"<<inOutData.name<<inOutData.inImgIndex<<inOutData.outImgIndex<<inOutData.dis;

    int disIndex = inOutData.outImgIndex - inOutData.inImgIndex + 1;
    oneImgDis = inOutData.dis/disIndex;

    disPixelW = bigImgW/sessionMile;
    disPixelH = bigImgH/cameraCount;
}

void Widget::readDiseaseExcel(QString path)
{
    char* excelPath = path.toLocal8Bit().data();
    QAxObject excel("Excel.Application");
    excel.setProperty("Visble",false);
    QAxObject* workbooks = excel.querySubObject("WorkBooks");
    QAxObject* workbook = workbooks->querySubObject("Open(QString,QVariant,QVariant)", path,3,true);
    QAxObject* worksheet1 = workbook->querySubObject("Worksheets(int)",1);

    QAxObject* usedRange = worksheet1->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    //Excel索引从1开始
    int intRowStart = usedRange->property("Row").toInt();
    int intColStart = usedRange->property("Column").toInt();
    int intRows = rows->property("Count").toInt();
    int intCols = columns->property("Count").toInt();
    qDebug()<<intRowStart<<intColStart<<intRows<<intCols;
    for(int i = intRowStart;i<intRowStart + intRows;i++){
        diseaseInfo temp;
        //赋初始值，防止提供的数据有错误，引起未知错误
        double imgTopPixel = 0;//小图像top对应段图像的像素值
        int sessionIndex = 0;
        double left = 0;//小图像left对应段图像的距离
        for(int j = intColStart;j<intColStart + intCols;j++)
        {
            QAxObject * range = worksheet1->querySubObject("Cells(int,int)", i, j );
            QVariant var = range->property("Value");
//            qDebug()<<var;
            if(j == 2)//原始图片名称
            {
//                temp.type = var.toString();
                temp.cameraID = var.toInt();
                imgTopPixel = (temp.cameraID-1)*disPixelH;
            }
            else if(j == 3)
            {
                temp.imageID = var.toInt();
                //距起始图像的张数
                int disIndex = temp.imageID - inOutData.inImgIndex;
                //图像左端距起始点距离
                double dis = disIndex*oneImgDis;
                //计算应该绘制的段图像索引
                sessionIndex = dis/sessionMile;
                //图像距离段图像的距离
                left = dis - sessionIndex*sessionMile;
            }
            else if(j == 13){//病害类型，需要修改
                temp.diseaseType = var.toString();
            }
            else if(j == 15){
                QString srcStr = var.toString();
                temp.diseaseData = srcStr;
                QStringList list = srcStr.split(";");
                //准备2个段图像病害数据
                sessionDisease temp1,temp2;
                bool bSplit = false;
                for(int i = 0;i<list.length() - 1;i++){//去掉最后一个
                    QString str2 = list[i];
                    QStringList list2 = str2.split(",");
                    if(list2.length() != 2){
                        qDebug()<<"length!=2";
                    }
                    int h = list2[1].toInt();
                    int w = list2[0].toInt();
                    //缩放转换
                    int x = (left + (w/smallImgW)*oneImgDis)*disPixelW;
                    int y = h/(smallImgH/disPixelH) + imgTopPixel;
                    //判断该图像是否被分割线一分为二
                    if(w<=bigImgW){
                        temp1.diseaseType = temp.diseaseType;
                        temp1.index = sessionIndex;
                        temp1.pts.append(QPointF(x,y));

                    }else{
                        bSplit = true;
                        temp2.diseaseType = temp.diseaseType;
                        temp2.index = sessionIndex + 1;
                        temp2.pts.append(QPointF(x,y));
                    }
                    if(i == list.length() - 2){//处理的最后一个数据点
                        if(bSplit){
                            sessionMap.insertMulti(temp1.index,temp1);
                            sessionMap.insertMulti(temp2.index,temp2);
                        }else{
                            sessionMap.insertMulti(temp1.index,temp1);
                        }
                    }
                }
            }
        }
    }
}

int Widget::getIndexByImgName(QString name)
{
    int temp;

    QStringList list = name.split("-");
    int len = list.length();
    QStringList list2 = list[len-1].split(".");
    temp = list2[0].toInt();
    return temp;
}

void Widget::InitParams()
{
    sessionMile = 50;
    cameraCount = 30;
    //段图像尺寸，可修改
    bigImgW = 11877;
    bigImgH = 4947;
    //假设原始图像宽对应这段图像的高
    smallImgW = 2048;
    smallImgH = 2560;
}

void Widget::ClearToRestart()
{
    ui->inOutLE->clear();
    ui->diseaseLE->clear();
    ui->remainLE->clear();
}

void Widget::Test0()
{
    Mat img = imread("150.jpg");
    if(img.empty()){
        qDebug()<<"imread failed!!!";
    }
    //创建一个名字为MyWindow的窗口
//    namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
    //在MyWindow的窗中中显示存储在img中的图片
    imshow("MyWindow", img);
    //等待直到有键按下
//    waitKey(0);
    //销毁MyWindow的窗口
    //    destroyWindow("MyWindow");
}

void Widget::Test1()
{
    QString root = QFileDialog::getExistingDirectory(this,"Open Dir",".");
    QStringList rootList = root.split("/");
    int len = rootList.length();
    QString name = rootList[len-1];
    qDebug()<<"rootList:"<<rootList[len-1];
    qDebug()<<"root:"<<root;
    QDir dir(root);
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0;i<list.length();i++){
        QString strName = list[i].fileName();
        QString strPath = list[i].filePath();
        qDebug()<<"###:"<<strName<<strPath<<list.length();
        QStringList nameList = strName.split(".");
        if(nameList[0] == name){
            qDebug()<<"@@@:"<<i;
        }
    }
}

void Widget::on_startDrawBtn_clicked()
{
    int sessionCount = inOutData.dis/sessionMile + 1;
    double lastMile = inOutData.dis - (sessionCount - 1)*sessionMile;
    int start = qRound((bigImgW/sessionMile)*lastMile);
    DrawThread* thread = new DrawThread(sessionCount);
    thread->m_lastImgStart = start;
    thread->m_lastImgEnd = bigImgW;
    connect(thread,SIGNAL(sendRemain(int,int)),this,SLOT(updateRemain(int,int)));
    thread->start();
}

void Widget::updateRemain(int count, int ms)
{
    if(count == -1){
        ui->label->setVisible(true);
    }
    else if(count == -2){
        ui->label->setVisible(false);
    }
    else {
        ui->remainLE->setText(QString::number(count));
        ui->useTimeLE->setText(QString::number(ms));
    }
}

void Widget::on_testBtn_clicked()
{
//    Test0();//测试使用opencv2.0版本函数库
    Test1();
}

void Widget::on_pushButton_2_clicked()
{
    QString root = QFileDialog::getExistingDirectory(this,"Open Dir",".");
    ui->tunnelDirLE->setText(root);
    QStringList rootList = root.split("/");
    int len = rootList.length();
    //获取隧道名称
    QString name = rootList[len-1];
    qDebug()<<"rootList:"<<rootList[len-1];
    qDebug()<<"root:"<<root;
    QDir dir(root);
    //准备过滤条件(文件扩展名)
    QStringList filters;
    filters << "*.xls" << "*.xlsx";
    QFileInfoList list = dir.entryInfoList(filters,QDir::Files);
    //获取进出洞隧道信息路径和病害信息路径
    QString inOutPath = "";
    QString diseasePath = "";
    for(int i = 0;i<list.length();i++){
        QString strName = list[i].fileName();//(格式)武邵高速上行叶坊隧道.xls
        QString strPath = list[i].filePath();
        qDebug()<<"###:"<<strName<<strPath<<list.length();
        QStringList nameList = strName.split(".");
        //查找与隧道名称同名的Excel文件
        if(nameList[0] == name){
            diseasePath = strPath;
        }else{
            inOutPath = strPath;
        }
    }
    //程序中，必须先读取进出洞隧道信息，再读取病害信息
    readInOutExcel(inOutPath);
    readDiseaseExcel(diseasePath);
}
