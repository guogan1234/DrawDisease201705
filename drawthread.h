#ifndef DRAWTHREAD_H
#define DRAWTHREAD_H

#include <QThread>

class DrawThread : public QThread
{
    Q_OBJECT
public:
    explicit DrawThread(QObject *parent = 0);
    DrawThread(int i,QObject *parent = 0);
    ~DrawThread();

    int m_lastImgStart;
    int m_lastImgEnd;

    QString tunnelNo;

signals:
    void sendRemain(int count,int ms);
    void workFinished();
public slots:

private:
    int m_count;

    int contoursDis;//区域性病害合并间距参数
    int linesDis;//线性病害合并间距参数

    double sessionMile;
    double tunnelArc;
    double bigImgW;
    double bigImgH;

    double tunnelMile;
    double onePixelW;
    double onePixelH;
    double onePixelArea;

    void tunnelNoToMile(QString no);
    QString tunnelMileToNo(double mile);
    void InitOnePixel();
    void InitParams();
    void run();

    bool CopyImages(int i);
    void CutLastImg(int start, int end);
    void CutLastImg2(int start, int end);
    void CutLastImg3(int start, int end);
    void CutLastImg4(int start, int end);
    void CutLastImg5(int start, int end);
    void CutLastImg6(int start, int end);
    void DrawDisease();

    //合并线性、区域病害
    void MergeDisease();

    //计算病害长度等
    void CalculateDisease();
    void CalculatePixelToMile(int count,double px,double py,double lenOrArea,bool flag);
    void ShowFinalDiseaseList();//测试数据结果使用

    //合并线性、区域图片
    void mergeImages();

    //根据病害类型名称查找病害类型缩写
    QString findDiseaseType_SX(QString type);
};

#endif // DRAWTHREAD_H
