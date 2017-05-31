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

signals:
    void sendRemain(int count,int ms);
public slots:

private:
    int m_count;

    int contoursDis;//区域性病害合并间距参数
    int linesDis;//线性病害合并间距参数

    void run();

    bool CopyImages(int i);
    void CutLastImg(int start, int end);
    void CutLastImg2(int start, int end);
    void DrawDisease();

    QString findDiseaseType_SX(QString type); 
};

#endif // DRAWTHREAD_H
