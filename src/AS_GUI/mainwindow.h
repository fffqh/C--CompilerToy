#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QClipboard>
#include <QFileDialog>
#include <QTextCodec>
#include <QProcess>
#include <QSize>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include "readfiledialog.h"
#include "../../LR/lr1.h"
#include "../../AS/toMips.h"

#define DOT_PATH ".\\Tree\\dot.exe"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_bnt_load_file_clicked();

    void on_bnt_reset_clicked();

    void on_bnt_start_clicked();

    void on_bnt_load_gplst_clicked();

    void on_bnt_img_up_clicked();

    void on_bnt_img_dw_clicked();

    void on_bnt_img_rst_clicked();

    void on_bnt_clear_info_clicked();

    void on_bnt_showfile_clicked();

    void on_bnt_showlr_clicked();

    void on_bnt_showinfo_clicked();

    void on_bnt_open_infodir_clicked();

    void on_bnt_img_open_clicked();

    void on_bnt_start_as_clicked();

    void on_bnt_copy_clicked();

private:
    Ui::MainWindow *ui;
    const std::string _treedot_path = ".\\lr1_tree.dot";
    const std::string _treepng_path = ".\\lr1_tree.png";
    std::string _file_path;
    std::string _gplst_path;
    QStandardItemModel  *   tb_model_for_AG;
    QStandardItemModel  *   tb_model_for_SI;
    QImage              *   img;

    ReadFileDialog * readfile_win;
    ReadFileDialog * readGplst_win;
    ReadFileDialog * readlog_win;

    void show_Action_Goto_tb();
    void show_Stack();
    void show_Tree();
    void show_log();

};

#endif // MAINWINDOW_H
