/********************************************************************************
** Form generated from reading UI file 'picform.ui'
**
** Created by: Qt User Interface Compiler version 5.12.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PICFORM_H
#define UI_PICFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_picForm
{
public:
    QLabel *picLabel;
    QPushButton *pushButton;

    void setupUi(QWidget *picForm)
    {
        if (picForm->objectName().isEmpty())
            picForm->setObjectName(QString::fromUtf8("picForm"));
        picForm->resize(675, 720);
        picLabel = new QLabel(picForm);
        picLabel->setObjectName(QString::fromUtf8("picLabel"));
        picLabel->setGeometry(QRect(40, 30, 571, 621));
        pushButton = new QPushButton(picForm);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(290, 680, 93, 28));

        retranslateUi(picForm);

        QMetaObject::connectSlotsByName(picForm);
    } // setupUi

    void retranslateUi(QWidget *picForm)
    {
        picForm->setWindowTitle(QApplication::translate("picForm", "\345\233\276\345\203\217", nullptr));
        picLabel->setText(QString());
        pushButton->setText(QApplication::translate("picForm", "\350\277\224\345\233\236", nullptr));
    } // retranslateUi

};

namespace Ui {
    class picForm: public Ui_picForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PICFORM_H
