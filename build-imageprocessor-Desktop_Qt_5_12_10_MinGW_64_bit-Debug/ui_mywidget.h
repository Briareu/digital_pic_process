/********************************************************************************
** Form generated from reading UI file 'mywidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYWIDGET_H
#define UI_MYWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_myWidget
{
public:
    QPushButton *m_upload;
    QPushButton *m_bmp;
    QPushButton *FT_btn;
    QPushButton *equal_btn;

    void setupUi(QWidget *myWidget)
    {
        if (myWidget->objectName().isEmpty())
            myWidget->setObjectName(QString::fromUtf8("myWidget"));
        myWidget->resize(488, 292);
        m_upload = new QPushButton(myWidget);
        m_upload->setObjectName(QString::fromUtf8("m_upload"));
        m_upload->setGeometry(QRect(40, 120, 93, 28));
        m_bmp = new QPushButton(myWidget);
        m_bmp->setObjectName(QString::fromUtf8("m_bmp"));
        m_bmp->setGeometry(QRect(140, 120, 93, 28));
        FT_btn = new QPushButton(myWidget);
        FT_btn->setObjectName(QString::fromUtf8("FT_btn"));
        FT_btn->setGeometry(QRect(240, 120, 93, 28));
        equal_btn = new QPushButton(myWidget);
        equal_btn->setObjectName(QString::fromUtf8("equal_btn"));
        equal_btn->setGeometry(QRect(340, 120, 93, 28));

        retranslateUi(myWidget);

        QMetaObject::connectSlotsByName(myWidget);
    } // setupUi

    void retranslateUi(QWidget *myWidget)
    {
        myWidget->setWindowTitle(QApplication::translate("myWidget", "\344\270\273\350\217\234\345\215\225", nullptr));
        m_upload->setText(QApplication::translate("myWidget", ".jpg;.png", nullptr));
        m_bmp->setText(QApplication::translate("myWidget", ".bmp", nullptr));
        FT_btn->setText(QApplication::translate("myWidget", "FT", nullptr));
        equal_btn->setText(QApplication::translate("myWidget", "equalize", nullptr));
    } // retranslateUi

};

namespace Ui {
    class myWidget: public Ui_myWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYWIDGET_H
