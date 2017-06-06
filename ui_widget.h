/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *pushButton;
    QPushButton *startDrawBtn;
    QFrame *line;
    QLabel *label_4;
    QLineEdit *remainLE;
    QFrame *line_2;
    QLabel *label;
    QPushButton *testBtn;
    QPushButton *pushButton_2;
    QLineEdit *tunnelDirLE;
    QWidget *hideWidget;
    QPushButton *readDisease;
    QLineEdit *diseaseLE;
    QPushButton *readInOut;
    QLineEdit *inOutLE;
    QLineEdit *useTimeLE;
    QLabel *label_5;
    QPushButton *pushButton_3;
    QLabel *label_2;
    QCheckBox *isFinishCBX;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(626, 135);
        pushButton = new QPushButton(Widget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(20, 10, 91, 21));
        startDrawBtn = new QPushButton(Widget);
        startDrawBtn->setObjectName(QStringLiteral("startDrawBtn"));
        startDrawBtn->setGeometry(QRect(20, 98, 90, 23));
        line = new QFrame(Widget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(10, 80, 601, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_4 = new QLabel(Widget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(130, 103, 54, 12));
        remainLE = new QLineEdit(Widget);
        remainLE->setObjectName(QStringLiteral("remainLE"));
        remainLE->setGeometry(QRect(180, 98, 113, 20));
        line_2 = new QFrame(Widget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(10, 30, 601, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        label = new QLabel(Widget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(530, 100, 91, 16));
        testBtn = new QPushButton(Widget);
        testBtn->setObjectName(QStringLiteral("testBtn"));
        testBtn->setGeometry(QRect(530, 10, 75, 23));
        pushButton_2 = new QPushButton(Widget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(20, 50, 91, 23));
        tunnelDirLE = new QLineEdit(Widget);
        tunnelDirLE->setObjectName(QStringLiteral("tunnelDirLE"));
        tunnelDirLE->setGeometry(QRect(130, 50, 461, 20));
        hideWidget = new QWidget(Widget);
        hideWidget->setObjectName(QStringLiteral("hideWidget"));
        hideWidget->setGeometry(QRect(10, 140, 611, 81));
        readDisease = new QPushButton(hideWidget);
        readDisease->setObjectName(QStringLiteral("readDisease"));
        readDisease->setGeometry(QRect(20, 50, 91, 23));
        diseaseLE = new QLineEdit(hideWidget);
        diseaseLE->setObjectName(QStringLiteral("diseaseLE"));
        diseaseLE->setGeometry(QRect(130, 50, 461, 20));
        readInOut = new QPushButton(hideWidget);
        readInOut->setObjectName(QStringLiteral("readInOut"));
        readInOut->setGeometry(QRect(20, 20, 91, 23));
        inOutLE = new QLineEdit(hideWidget);
        inOutLE->setObjectName(QStringLiteral("inOutLE"));
        inOutLE->setGeometry(QRect(130, 20, 461, 20));
        useTimeLE = new QLineEdit(Widget);
        useTimeLE->setObjectName(QStringLiteral("useTimeLE"));
        useTimeLE->setGeometry(QRect(380, 98, 113, 20));
        label_5 = new QLabel(Widget);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(300, 100, 81, 16));
        pushButton_3 = new QPushButton(Widget);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(230, 10, 75, 23));
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(132, 14, 54, 12));
        isFinishCBX = new QCheckBox(Widget);
        isFinishCBX->setObjectName(QStringLiteral("isFinishCBX"));
        isFinishCBX->setGeometry(QRect(130, 13, 111, 16));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", 0));
        pushButton->setText(QApplication::translate("Widget", "\345\215\225\344\270\252\351\232\247\351\201\223\345\244\204\347\220\206", 0));
        startDrawBtn->setText(QApplication::translate("Widget", "\345\274\200\345\247\213\347\273\230\345\210\266\347\227\205\345\256\263", 0));
        label_4->setText(QApplication::translate("Widget", "\345\211\251\344\275\231\351\207\217\357\274\232", 0));
        label->setText(QApplication::translate("Widget", "\345\233\276\345\203\217\345\244\215\345\210\266\344\270\255...", 0));
        testBtn->setText(QApplication::translate("Widget", "\346\265\213\350\257\225", 0));
        pushButton_2->setText(QApplication::translate("Widget", "\351\200\211\346\213\251\351\232\247\351\201\223\347\233\256\345\275\225", 0));
        readDisease->setText(QApplication::translate("Widget", "\350\257\273\345\217\226\347\227\205\345\256\263\344\277\241\346\201\257", 0));
        readInOut->setText(QApplication::translate("Widget", "\350\257\273\345\217\226\350\277\233\345\207\272\344\277\241\346\201\257", 0));
        label_5->setText(QApplication::translate("Widget", "\345\215\225\346\235\241\350\200\227\346\227\266(ms)\357\274\232", 0));
        pushButton_3->setText(QApplication::translate("Widget", "\345\255\230\345\205\245Excel", 0));
        label_2->setText(QString());
        isFinishCBX->setText(QApplication::translate("Widget", "\346\230\257\345\220\246\345\244\204\347\220\206\345\256\214\346\210\220", 0));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
