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

    void run();

    bool CopyImages(int i);
    void CutLastImg(int start, int end);
    void DrawDisease();

};

#endif // DRAWTHREAD_H
