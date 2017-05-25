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

private:
    Ui::Widget *ui;

    void readExcel(QString path,int flag);

    void readInOutExcel(QString path);

    void readDiseaseExcel(QString path);

    int getIndexByImgName(QString name);

    double oneImgDis;
    double disPixelW;//每个宽度距离的像素值
    double disPixelH;//每个相机图像高度的像素值
    double bigImgW;//段图像的宽度像素值
    double bigImgH;//段图像的高度像素值
    double smallImgW;//原始小图像的宽度像素值
    double smallImgH;//原始小图像的高度像素值
    //初始化
    int sessionMile;
    int cameraCount;

    void InitParams();

    //清空重新处理
    void ClearToRestart();

    //测试代码
    void Test0();
    void Test1();
};

#endif // WIDGET_H
