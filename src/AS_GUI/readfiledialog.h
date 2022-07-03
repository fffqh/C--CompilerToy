#ifndef READFILEDIALOG_H
#define READFILEDIALOG_H

#include <QDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

namespace Ui {
class ReadFileDialog;
}

class ReadFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReadFileDialog(QWidget *parent = 0, QString file_path="");
    ~ReadFileDialog();

private:
    Ui::ReadFileDialog *ui;
    QString fpath;

    void showfile();
};

#endif // READFILEDIALOG_H
