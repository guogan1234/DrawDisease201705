#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void on_readInOut_clicked();

    void on_readDisease_clicked();

    void on_startDrawBtn_clicked();

    void updateRemain(int count,int ms);

    void on_testBtn_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void threadWorkFinish();

private:
    Ui::Widget *ui;

    void readExcel(QString path,int flag);

    void readInOutExcel(QString path);

    void readDiseaseExcel(QString path);

    void readTunnelMileExcel();
    void compareTunnelMile(QString start,QString end);
    double tunnelNoToMile(QString no);

    int getIndexByImgName(QString name);

    double oneImgDis;
    double disPixelW;//段图像每个像素值的宽度值
    double disPixelH;//每个相机图像高度所占段图像的像素值
    double bigImgW;//段图像的宽度像素值
    double bigImgH;//段图像的高度像素值
    double smallImgW;//原始小图像的宽度像素值
    double smallImgH;//原始小图像的高度像素值
    //初始化
    int sessionMile;
    int cameraCount;

    QString theTunnelMile;

    void InitParams();

    //清空重新处理
    void ClearToRestart();

    void FillDiseaseType();

    //测试代码
    void Test0();
    void Test1();
};

#endif // WIDGET_H
