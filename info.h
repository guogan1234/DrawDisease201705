#ifndef INFO_H
#define INFO_H

#include <QPointF>

struct InOutInfo{
    QString name;
    int inImgIndex;
    int outImgIndex;
    double dis;
};

struct diseaseInfo{
    int cameraID;
    int imageID;
    QString diseaseType;
    QString diseaseData;
};

struct sessionDisease{
    QString diseaseType;
    //输出
    int index;//第几段图像
    QList<QPointF> pts;
};

#endif // INFO_H
