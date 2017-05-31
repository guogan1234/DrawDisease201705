#ifndef INFO_H
#define INFO_H

#include <QPointF>

struct InOutInfo{
    QString name;
    int inImgIndex;
    int outImgIndex;
    double dis;
};

//原始病害信息
struct diseaseInfo{
    int cameraID;
    int imageID;
    QString diseaseType;
    QString diseaseData;
};

//绘制时需要的病害绘制信息
struct sessionDisease{
    QString diseaseType;
    //输出
    int index;//第几段图像
    QList<QPointF> pts;
};

struct diseaseType{
    int id;
    QString type;
    QString type_SX;
};

#endif // INFO_H
