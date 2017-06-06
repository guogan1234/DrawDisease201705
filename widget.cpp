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

QHash<int,diseaseType> typeHash;
QHash<int,diseaseType>::iterator typeHashIter;

//最终病害信息存入Excel
extern bool finalOK;
extern QList<finalDiseaseInfo> finalDiseaseList;

//隧道起始里程信息
QList<tunnelMileInfo> tunnelMileList;

//隧道上下行标识
bool isMinToMax;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    qDebug()<<"1111";
    ui->setupUi(this);

    qDebug()<<"2222";
    InitParams();
    qDebug()<<"3333";
    FillDiseaseType();
    ui->label->setVisible(false);
//    ui->testBtn->setVisible(false);
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

    ui->isFinishCBX->setChecked(false);
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

    int disIndex = qAbs(inOutData.outImgIndex - inOutData.inImgIndex) + 1;
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
                int disIndex = qAbs(temp.imageID - inOutData.inImgIndex);
                //图像左端距起始点距离
                double dis = disIndex*oneImgDis;
                //计算应该绘制的段图像索引
                sessionIndex = dis/sessionMile;
                //图像距离段图像的距离
                left = dis - sessionIndex*sessionMile;
            }
            else if(j == 4){//病害类型
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
                    //缩放转换，小图像病害像素转换为段图像对应的像素
                    int x = (left + (w/smallImgW)*oneImgDis)*disPixelW;
                    int y = h/(smallImgH/disPixelH) + imgTopPixel;
                    //判断该图像是否被分割线一分为二
                    if(x<=bigImgW){
                        temp1.diseaseType = temp.diseaseType;
                        temp1.index = sessionIndex;
                        temp1.pts.append(QPointF(x,y));

                    }else{
                        bSplit = true;
                        temp2.diseaseType = temp.diseaseType;
                        temp2.index = sessionIndex + 1;
                        temp2.pts.append(QPointF(x-bigImgW,y));
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

void Widget::readTunnelMileExcel()
{
    QString projectPath = QDir::currentPath();
    projectPath += "/Data/tunnelMile.xlsx";

    QAxObject excel("Excel.Application");
    excel.setProperty("Visble",false);
    QAxObject* workbooks = excel.querySubObject("WorkBooks");
    QAxObject* workbook = workbooks->querySubObject("Open(QString,QVariant,QVariant)",projectPath,3,true);
    QAxObject* worksheet1 = workbook->querySubObject("Worksheets(int)",1);

    QAxObject* usedRange = worksheet1->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    //Excel索引从1开始
    int intRowStart = usedRange->property("Row").toInt();
    int intColStart = usedRange->property("Column").toInt();
    int intRows = rows->property("Count").toInt();
    int intCols = columns->property("Count").toInt();
    qDebug()<<"FillDiseaseType:"<<intRowStart<<intColStart<<intRows<<intCols;
    for(int i = intRowStart;i<intRowStart + intRows;i++){
        tunnelMileInfo temp;
        for(int j = intColStart;j<intColStart + intCols;j++)
        {
            QAxObject * range = worksheet1->querySubObject("Cells(int,int)", i, j );
            QVariant var = range->property("Value");
            if(j == 1){
                temp.tunnelName = var.toString();
            }
            else if(j==2){
                temp.nameParam1 = var.toString();
            }
            else if(j == 3){
                temp.nameParam2 = var.toString();
            }
            else if(j == 4){
                temp.startMile = var.toString();
            }
            else if(j== 5){
                temp.endMile = var.toString();
            }
        }
        temp.finalTunnelName = temp.tunnelName + "-" + temp.nameParam1 + "-" + temp.nameParam2;
        tunnelMileList.append(temp);
    }
}

void Widget::compareTunnelMile(QString start, QString end)
{
    double d_start = 0;
    d_start = tunnelNoToMile(start);
    double d_end;
    d_end = tunnelNoToMile(end);
    if(d_start < d_end){
        isMinToMax = true;
    }else{
        isMinToMax = false;
    }
}

double Widget::tunnelNoToMile(QString no)
{
    double temp;
    QStringList list = no.split("+");
    int k = list[0].right(3).toInt();
    double m = list[1].toDouble();
    temp = k*1000.0 + m;
    return temp;
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

void Widget::FillDiseaseType()
{
    qDebug()<<"FillDiseaseType...";
    QString projectPath = QDir::currentPath();
    projectPath += "/Data/diseaseType.xlsx";
    qDebug()<<"appPath:"<<projectPath;
//    char* path = "F:/QtMyProjectTest/DrawDisease201705/Data/diseaseType.xlsx";
//    char* ch_projectPath = projectPath.toLocal8Bit().data();
//    char* cat = "/Data/diseaseType.xlsx";
//    strcat(ch_projectPath,cat);
//    char path[128];
    QAxObject excel("Excel.Application");
    excel.setProperty("Visble",false);
    QAxObject* workbooks = excel.querySubObject("WorkBooks");
    QAxObject* workbook = workbooks->querySubObject("Open(QString,QVariant,QVariant)",projectPath,3,true);
    QAxObject* worksheet1 = workbook->querySubObject("Worksheets(int)",1);

    QAxObject* usedRange = worksheet1->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    //Excel索引从1开始
    int intRowStart = usedRange->property("Row").toInt();
    int intColStart = usedRange->property("Column").toInt();
    int intRows = rows->property("Count").toInt();
    int intCols = columns->property("Count").toInt();
    qDebug()<<"FillDiseaseType:"<<intRowStart<<intColStart<<intRows<<intCols;
    for(int i = intRowStart;i<intRowStart + intRows;i++){
        diseaseType temp;
        for(int j = intColStart;j<intColStart + intCols;j++)
        {
            QAxObject * range = worksheet1->querySubObject("Cells(int,int)", i, j );
            QVariant var = range->property("Value");
            if(j == 1){
                temp.id = var.toInt();
            }
            else if(j==2){
                temp.type = var.toString();
            }
            else if(j == 3){
                temp.type_SX = var.toString();
            }
        }
        typeHash.insert(temp.id,temp);
    }
}

void Widget::Test0()
{
//    char ch[128] = "./150.jpg";
    QString qStr = "F:/QtMyProjectTest/DrawDisease201705/test.jpg";
//    string str("./150.jpg");
//    string str = qStr.toStdString();
//    cout<<"@@@"<<str;
//    Mat img = imread("F://QtMyProjectTest//DrawDisease201705//150.jpg");//编译通过，运行出错，无法识别char*常量字符串，读取图像失败
     Mat img = imread(qStr.toLocal8Bit().data());//编译通过，运行出错，无法识别char*常量字符串转换的string,，读取图像失败
     if(img.empty()){
         qDebug()<<"Load failed!!!";
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
    //段图像总张数
    int sessionCount = inOutData.dis/sessionMile + 1;
    //最后一张段图像的有效距离
    double lastMile = inOutData.dis - (sessionCount - 1)*sessionMile;
    int start = qRound((bigImgW/sessionMile)*lastMile);
    DrawThread* thread = new DrawThread(sessionCount);
    thread->m_lastImgStart = start;
    thread->m_lastImgEnd = bigImgW;
    thread->tunnelNo = theTunnelMile;
    connect(thread,SIGNAL(sendRemain(int,int)),this,SLOT(updateRemain(int,int)));
    connect(thread,SIGNAL(workFinished()),this,SLOT(threadWorkFinish()));
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
    Test0();//测试使用opencv2.0版本函数库
//    Test1();
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
    //读取隧道里程信息
    readTunnelMileExcel();
    //匹配查找当前隧道的起始里程信息
    for(int i = 0;i<tunnelMileList.length();i++){
//        qDebug()<<"@@@:"<<name<<tunnelMileList[i].finalTunnelName;
        if(name == tunnelMileList[i].finalTunnelName){
            //区分隧道上下行
            QString start = tunnelMileList[i].startMile;
            QString end = tunnelMileList[i].endMile;
            compareTunnelMile(start,end);
            qDebug()<<"###:"<<i;
            theTunnelMile = tunnelMileList[i].startMile;
            return;
        }
    }
    qDebug()<<"Not Find!!!";
}

void Widget::on_pushButton_3_clicked()
{
    if(!finalOK){
        return;
    }
    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", false);
    QAxObject * workbooks = excel.querySubObject("WorkBooks");
    workbooks->dynamicCall("Add");
    QAxObject * workbook = excel.querySubObject("ActiveWorkBook");
    QAxObject * worksheets = workbook->querySubObject("WorkSheets");
    int intCount = worksheets->property("Count").toInt();
    qDebug()<<intCount;
    QAxObject * worksheet = workbook->querySubObject("Worksheets(int)", 1);

    for(int i = 0;i<finalDiseaseList.length();i++)
    {
        QString no = finalDiseaseList[i].startMile;
        bool b = finalDiseaseList[i].IsMin;
        int i_b = 0;
        if(b){
            i_b = 1;
        }
        double d3 = finalDiseaseList[i].distanceRight;
        double d4 = finalDiseaseList[i].length;
        double d5 = finalDiseaseList[i].area;

//        double x = pt.x();
//        double y = pt.y();

        QString param1,param2,param3,param4,param5;
        param1 = "Cells(" + QString::number(i+1) + ",1)";
        param2 = "Cells(" + QString::number(i+1) + ",2)";
        param3 = "Cells(" + QString::number(i+1) + ",3)";
        param4 = "Cells(" + QString::number(i+1) + ",4)";
        param5 = "Cells(" + QString::number(i+1) + ",5)";
        QByteArray baX;
        QByteArray baY;
        baX = param1.toLatin1();
        baY = param2.toLatin1();
        char* chX= NULL;
        char* chY= NULL;
        chX = baX.data();
        chY = baY.data();

        char* ch3 = NULL;
        char* ch4= NULL;
        char* ch5= NULL;
        QByteArray ba3;
        QByteArray ba4;
        QByteArray ba5;
        ba3 = param3.toLatin1();
        ch3 = ba3.data();
        ba4 = param4.toLatin1();
        ch4 = ba4.data();
        ba5 = param5.toLatin1();
        ch5 = ba5.data();

        QAxObject * rangeX = worksheet->querySubObject(chX);
        bool b1 = rangeX->setProperty("Value",QVariant(no));

        QAxObject * rangeY = worksheet->querySubObject(chY);
        bool b2 = rangeY->setProperty("Value",QVariant(i_b));

        QAxObject * range3 = worksheet->querySubObject(ch3);
        bool b3 = range3->setProperty("Value",QVariant(d3));

        QAxObject * range4 = worksheet->querySubObject(ch4);
        bool b4 = range4->setProperty("Value",QVariant(d4));

        QAxObject * range5 = worksheet->querySubObject(ch5);
        bool b5 = range5->setProperty("Value",QVariant(d5));

        qDebug()<<"###:"<<i;
    }
    QString fileName;
    fileName = "F:\\QtMyProjectTest\\DrawDisease201705\\1.xlsx";

    //保存时，提供的参数路径，需为Window格式，且不能为相对路径，否则不会生成
    workbook->dynamicCall("SaveAs (const QString&)", fileName);
    workbook->dynamicCall("Close (Boolean)", true);
    excel.dynamicCall("Quit (void)");
}

void Widget::threadWorkFinish()
{
    ui->isFinishCBX->setChecked(true);
}
