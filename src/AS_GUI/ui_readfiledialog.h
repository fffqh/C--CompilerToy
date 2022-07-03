/********************************************************************************
** Form generated from reading UI file 'readfiledialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_READFILEDIALOG_H
#define UI_READFILEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_ReadFileDialog
{
public:
    QTextBrowser *txt_file;

    void setupUi(QDialog *ReadFileDialog)
    {
        if (ReadFileDialog->objectName().isEmpty())
            ReadFileDialog->setObjectName(QStringLiteral("ReadFileDialog"));
        ReadFileDialog->resize(629, 710);
        txt_file = new QTextBrowser(ReadFileDialog);
        txt_file->setObjectName(QStringLiteral("txt_file"));
        txt_file->setGeometry(QRect(10, 10, 611, 691));

        retranslateUi(ReadFileDialog);

        QMetaObject::connectSlotsByName(ReadFileDialog);
    } // setupUi

    void retranslateUi(QDialog *ReadFileDialog)
    {
        ReadFileDialog->setWindowTitle(QApplication::translate("ReadFileDialog", "Dialog", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ReadFileDialog: public Ui_ReadFileDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_READFILEDIALOG_H
