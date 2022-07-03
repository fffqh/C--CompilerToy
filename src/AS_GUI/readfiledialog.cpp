#include "readfiledialog.h"
#include "ui_readfiledialog.h"

ReadFileDialog::ReadFileDialog(QWidget *parent, QString file_path) :
    QDialog(parent),
    ui(new Ui::ReadFileDialog)
{
    ui->setupUi(this);
    this->fpath = file_path;
    this->showfile();
}
ReadFileDialog::~ReadFileDialog()
{
    delete ui;
}

void ReadFileDialog::showfile()
{
    if(this->fpath == "")
        return;

    QFile file(this->fpath);

    //--打开文件成功
    if (file.open(QIODevice ::ReadOnly | QIODevice ::Text))
    {
        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            //---QtextEdit按行显示文件内容
            this->ui->txt_file->append(textStream.readLine());
        }
    }
    else	//---打开文件失败
    {
        QMessageBox ::information(NULL, NULL, "open file error");
    }
}
