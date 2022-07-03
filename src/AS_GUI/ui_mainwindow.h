/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTableView *tbv_ActionGoto;
    QTabWidget *tabWidget;
    QWidget *tab;
    QTableView *tbv_result;
    QWidget *tab_2;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout;
    QLabel *lb_tree_img;
    QPushButton *bnt_img_up;
    QPushButton *bnt_img_dw;
    QPushButton *bnt_img_rst;
    QPushButton *bnt_img_open;
    QWidget *tab_3;
    QPushButton *bnt_start_as;
    QTextBrowser *txt_mips;
    QStackedWidget *stacked_w;
    QListWidget *lst_cb;
    QPushButton *bnt_copy;
    QGroupBox *groupBox;
    QPushButton *bnt_load_file;
    QPushButton *bnt_start;
    QPushButton *bnt_showlr;
    QPushButton *bnt_reset;
    QPushButton *bnt_showfile;
    QPushButton *bnt_load_gplst;
    QLabel *lb_file_path;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QGroupBox *groupBox_2;
    QTextBrowser *txt_info;
    QPushButton *bnt_clear_info;
    QPushButton *bnt_open_infodir;
    QPushButton *bnt_showinfo;
    QTextBrowser *txt_tokenlst;
    QTextBrowser *txt_GPlst;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1306, 926);
        MainWindow->setStyleSheet(QStringLiteral("background-color: rgb(247, 255, 244);"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tbv_ActionGoto = new QTableView(centralWidget);
        tbv_ActionGoto->setObjectName(QStringLiteral("tbv_ActionGoto"));
        tbv_ActionGoto->setGeometry(QRect(10, 360, 691, 291));
        tbv_ActionGoto->setStyleSheet(QLatin1String("border:1px solid green;\n"
"\n"
""));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(730, 20, 551, 641));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        tbv_result = new QTableView(tab);
        tbv_result->setObjectName(QStringLiteral("tbv_result"));
        tbv_result->setGeometry(QRect(0, 0, 541, 611));
        tbv_result->setStyleSheet(QLatin1String("border:1px solid green;\n"
"\n"
""));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        scrollArea = new QScrollArea(tab_2);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setGeometry(QRect(0, 0, 541, 591));
        scrollArea->setStyleSheet(QStringLiteral(""));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 539, 589));
        gridLayout = new QGridLayout(scrollAreaWidgetContents);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lb_tree_img = new QLabel(scrollAreaWidgetContents);
        lb_tree_img->setObjectName(QStringLiteral("lb_tree_img"));

        gridLayout->addWidget(lb_tree_img, 0, 0, 1, 1);

        scrollArea->setWidget(scrollAreaWidgetContents);
        bnt_img_up = new QPushButton(tab_2);
        bnt_img_up->setObjectName(QStringLiteral("bnt_img_up"));
        bnt_img_up->setGeometry(QRect(20, 590, 51, 21));
        bnt_img_up->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 1px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        bnt_img_dw = new QPushButton(tab_2);
        bnt_img_dw->setObjectName(QStringLiteral("bnt_img_dw"));
        bnt_img_dw->setGeometry(QRect(70, 590, 51, 21));
        bnt_img_dw->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 1px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        bnt_img_rst = new QPushButton(tab_2);
        bnt_img_rst->setObjectName(QStringLiteral("bnt_img_rst"));
        bnt_img_rst->setGeometry(QRect(120, 590, 51, 21));
        bnt_img_rst->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 1px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        bnt_img_open = new QPushButton(tab_2);
        bnt_img_open->setObjectName(QStringLiteral("bnt_img_open"));
        bnt_img_open->setGeometry(QRect(420, 590, 101, 21));
        bnt_img_open->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 1px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        bnt_start_as = new QPushButton(tab_3);
        bnt_start_as->setObjectName(QStringLiteral("bnt_start_as"));
        bnt_start_as->setGeometry(QRect(20, 10, 131, 31));
        bnt_start_as->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        txt_mips = new QTextBrowser(tab_3);
        txt_mips->setObjectName(QStringLiteral("txt_mips"));
        txt_mips->setGeometry(QRect(20, 50, 511, 231));
        stacked_w = new QStackedWidget(tab_3);
        stacked_w->setObjectName(QStringLiteral("stacked_w"));
        stacked_w->setGeometry(QRect(140, 300, 391, 301));
        lst_cb = new QListWidget(tab_3);
        lst_cb->setObjectName(QStringLiteral("lst_cb"));
        lst_cb->setGeometry(QRect(20, 300, 111, 301));
        bnt_copy = new QPushButton(tab_3);
        bnt_copy->setObjectName(QStringLiteral("bnt_copy"));
        bnt_copy->setGeometry(QRect(400, 10, 131, 31));
        bnt_copy->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        tabWidget->addTab(tab_3, QString());
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 20, 151, 301));
        bnt_load_file = new QPushButton(groupBox);
        bnt_load_file->setObjectName(QStringLiteral("bnt_load_file"));
        bnt_load_file->setGeometry(QRect(10, 30, 131, 51));
        bnt_load_file->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        bnt_start = new QPushButton(groupBox);
        bnt_start->setObjectName(QStringLiteral("bnt_start"));
        bnt_start->setGeometry(QRect(10, 140, 131, 31));
        bnt_start->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        bnt_showlr = new QPushButton(groupBox);
        bnt_showlr->setObjectName(QStringLiteral("bnt_showlr"));
        bnt_showlr->setGeometry(QRect(10, 260, 131, 31));
        bnt_showlr->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        bnt_reset = new QPushButton(groupBox);
        bnt_reset->setObjectName(QStringLiteral("bnt_reset"));
        bnt_reset->setGeometry(QRect(10, 180, 131, 31));
        bnt_reset->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        bnt_showfile = new QPushButton(groupBox);
        bnt_showfile->setObjectName(QStringLiteral("bnt_showfile"));
        bnt_showfile->setGeometry(QRect(10, 230, 131, 31));
        bnt_showfile->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        bnt_load_gplst = new QPushButton(groupBox);
        bnt_load_gplst->setObjectName(QStringLiteral("bnt_load_gplst"));
        bnt_load_gplst->setGeometry(QRect(10, 80, 131, 51));
        bnt_load_gplst->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}"));
        lb_file_path = new QLabel(centralWidget);
        lb_file_path->setObjectName(QStringLiteral("lb_file_path"));
        lb_file_path->setGeometry(QRect(170, 20, 531, 20));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(260, 330, 61, 16));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(490, 330, 111, 16));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(280, 660, 141, 16));
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(950, 670, 121, 16));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 690, 1271, 171));
        txt_info = new QTextBrowser(groupBox_2);
        txt_info->setObjectName(QStringLiteral("txt_info"));
        txt_info->setGeometry(QRect(10, 20, 1091, 141));
        txt_info->setStyleSheet(QLatin1String("background-color: rgb(246, 255, 251);\n"
"border:1px solid green;\n"
"\n"
""));
        bnt_clear_info = new QPushButton(groupBox_2);
        bnt_clear_info->setObjectName(QStringLiteral("bnt_clear_info"));
        bnt_clear_info->setGeometry(QRect(1130, 20, 121, 28));
        bnt_clear_info->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        bnt_open_infodir = new QPushButton(groupBox_2);
        bnt_open_infodir->setObjectName(QStringLiteral("bnt_open_infodir"));
        bnt_open_infodir->setGeometry(QRect(1130, 100, 121, 51));
        bnt_open_infodir->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        bnt_showinfo = new QPushButton(groupBox_2);
        bnt_showinfo->setObjectName(QStringLiteral("bnt_showinfo"));
        bnt_showinfo->setGeometry(QRect(1130, 60, 121, 28));
        bnt_showinfo->setStyleSheet(QLatin1String("QPushButton{\n"
"border-color:green;\n"
"border-width: 2px;\n"
"border-style: solid;\n"
"selection-color: rgb(107, 170, 128);\n"
"}\n"
"QPushButton:hover{\n"
"	color: rgb(0, 0, 0);\n"
"	border-color: green;\n"
"    background-color: rgb(217, 255, 200);\n"
"}\n"
""));
        txt_tokenlst = new QTextBrowser(centralWidget);
        txt_tokenlst->setObjectName(QStringLiteral("txt_tokenlst"));
        txt_tokenlst->setGeometry(QRect(170, 50, 231, 271));
        txt_tokenlst->setStyleSheet(QLatin1String("border:1px solid green;\n"
"\n"
""));
        txt_tokenlst->setTabStopWidth(40);
        txt_GPlst = new QTextBrowser(centralWidget);
        txt_GPlst->setObjectName(QStringLiteral("txt_GPlst"));
        txt_GPlst->setGeometry(QRect(405, 50, 291, 271));
        txt_GPlst->setStyleSheet(QLatin1String("border:1px solid green;\n"
"\n"
""));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1306, 26));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(2);
        stacked_w->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        tab->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_ACCESSIBILITY
        tab->setAccessibleName(QString());
#endif // QT_NO_ACCESSIBILITY
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "\350\257\255\346\263\225\345\210\206\346\236\220\346\240\210", Q_NULLPTR));
        lb_tree_img->setText(QString());
        bnt_img_up->setText(QApplication::translate("MainWindow", "\346\224\276\345\244\247", Q_NULLPTR));
        bnt_img_dw->setText(QApplication::translate("MainWindow", "\347\274\251\345\260\217", Q_NULLPTR));
        bnt_img_rst->setText(QApplication::translate("MainWindow", "reset", Q_NULLPTR));
        bnt_img_open->setText(QApplication::translate("MainWindow", " \346\211\223\345\274\200\345\233\276\347\211\207\346\226\207\344\273\266", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "\350\257\255\346\263\225\345\210\206\346\236\220\346\240\221", Q_NULLPTR));
        bnt_start_as->setText(QApplication::translate("MainWindow", "\347\224\237\346\210\220\346\261\207\347\274\226\344\273\243\347\240\201", Q_NULLPTR));
        bnt_copy->setText(QApplication::translate("MainWindow", "\345\244\215\345\210\266\344\273\243\347\240\201\345\210\260\345\211\252\350\264\264\346\235\277", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("MainWindow", "\347\224\237\346\210\220\345\217\257\346\211\247\350\241\214\346\261\207\347\274\226\344\273\243\347\240\201", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("MainWindow", "\346\223\215\344\275\234\345\217\260", Q_NULLPTR));
        bnt_load_file->setText(QApplication::translate("MainWindow", "\344\273\216\346\226\207\344\273\266\345\257\274\345\205\245\n"
"\346\272\220\346\226\207\344\273\266", Q_NULLPTR));
        bnt_start->setText(QApplication::translate("MainWindow", " \345\274\200\345\247\213\350\257\255\346\263\225\345\210\206\346\236\220", Q_NULLPTR));
        bnt_showlr->setText(QApplication::translate("MainWindow", " \346\237\245\347\234\213\345\275\223\345\211\215\346\226\207\346\263\225", Q_NULLPTR));
        bnt_reset->setText(QApplication::translate("MainWindow", "\351\207\215\347\275\256", Q_NULLPTR));
        bnt_showfile->setText(QApplication::translate("MainWindow", " \346\237\245\347\234\213\346\272\220\346\226\207\344\273\266", Q_NULLPTR));
        bnt_load_gplst->setText(QApplication::translate("MainWindow", "\344\273\216\346\226\207\344\273\266\345\257\274\345\205\245\n"
"\346\226\207\346\263\225\346\226\207\344\273\266", Q_NULLPTR));
        lb_file_path->setText(QApplication::translate("MainWindow", "\346\234\252\345\257\274\345\205\245\346\272\220\346\226\207\344\273\266...", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", " \347\254\246\345\217\267\350\241\250", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", " \346\226\207\346\263\225\344\272\247\347\224\237\345\274\217\350\241\250", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "\346\255\243\350\247\204 LR \350\257\255\346\263\225\345\210\206\346\236\220\350\241\250", Q_NULLPTR));
        label_5->setText(QApplication::translate("MainWindow", "LR1 \350\257\255\346\263\225\345\210\206\346\236\220\347\273\223\346\236\234", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "\346\227\245\345\277\227\350\276\223\345\207\272", Q_NULLPTR));
        bnt_clear_info->setText(QApplication::translate("MainWindow", "\346\270\205\347\251\272\346\227\245\345\277\227\351\235\242\346\235\277", Q_NULLPTR));
        bnt_open_infodir->setText(QApplication::translate("MainWindow", "\346\211\223\345\274\200\346\227\245\345\277\227\346\226\207\344\273\266\n"
"\346\211\200\345\234\250\346\226\207\344\273\266\345\244\271", Q_NULLPTR));
        bnt_showinfo->setText(QApplication::translate("MainWindow", "\346\237\245\347\234\213\346\227\245\345\277\227\346\226\207\344\273\266", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
