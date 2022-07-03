#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//或者"GBK",不分大小写
    QTextCodec::setCodecForLocale(codec);

    ui->setupUi(this);
    this->_file_path = "";
    this->_gplst_path = "";
    this->tb_model_for_AG = NULL;
    this->tb_model_for_SI = NULL;
    this->img = NULL;

    this->readfile_win = NULL;
    this->readGplst_win = NULL;
    this->readlog_win = NULL;
    this->ui->bnt_start_as->setDisabled(true);
    this->setWindowTitle(QString::fromStdString("简易编译器-1851746-范千惠"));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(this->tb_model_for_AG) delete this->tb_model_for_AG;
    if(this->tb_model_for_SI) delete this->tb_model_for_SI;
    if(this->img) delete this->img;
    if(this->readfile_win) delete readfile_win;
    if(this->readGplst_win) delete readGplst_win;
    if(this->readlog_win) delete readlog_win;
}

void MainWindow::on_bnt_load_file_clicked()
{
    QDir dir(QDir::currentPath());
    dir.cdUp();
    std::string rfile_path = (
        QFileDialog::getOpenFileName(this, tr("Select File"), dir.absolutePath())
    ).toStdString();

    if(rfile_path != ""){
        this->_file_path = rfile_path;
        this->ui->lb_file_path->setText(QString::fromStdString(rfile_path));
    }
}
void MainWindow::on_bnt_load_gplst_clicked()
{
    QDir dir(QDir::currentPath());
    dir.cdUp();
    std::string rfile_path = (
        QFileDialog::getOpenFileName(this, tr("Select GPList File"), dir.absolutePath())
    ).toStdString();

    if(rfile_path != ""){
        this->_gplst_path = rfile_path;
    }
}

void MainWindow::on_bnt_reset_clicked()
{
    this->_file_path = "";
    this->_gplst_path = "";
    this->ui->lb_file_path->setText(QString::fromStdString("未导入源文件..."));
    this->ui->lb_tree_img->clear();
    this->ui->txt_mips->clear();
    this->ui->bnt_start_as->setDisabled(true);
    if(this->tb_model_for_AG){
        this->tb_model_for_AG->clear();
        delete this->tb_model_for_AG;
        this->tb_model_for_AG = NULL;
    }
    if(this->tb_model_for_SI){
        this->tb_model_for_SI->clear();
        delete this->tb_model_for_SI;
        this->tb_model_for_SI = NULL;
    }
    if(this->img){
        delete this->img;
        this->img = NULL;
    }
}

void MainWindow::on_bnt_start_clicked()
{
    if((this->_file_path == "") || (this->_gplst_path == "")){
        return;
    }

    this->setWindowTitle(QString::fromStdString("简易编译器(请等待...)"));
    if(lalr_run(this->_file_path, this->_gplst_path)){
        QMessageBox::information(this,"成功","语法分析完成!",QMessageBox::Ok);
        //展示符号表
        QString token_lst = QString::fromStdString(get_token_lst());
        this->ui->txt_tokenlst->setText(token_lst);
        //展示文法产生式
        QString GP_lst = QString::fromStdString(get_GP_lst());
        this->ui->txt_GPlst->setText(GP_lst);
        //展示LR1语法分析表
        show_Action_Goto_tb();

        //栈表示展示
        show_Stack();
        //语法分析树展示
        show_Tree();
        //日志显示
        show_log();

        this->ui->bnt_start_as->setDisabled(false);
    }else{
        QMessageBox::information(this,"失败","语法分析失败",QMessageBox::Ok);
        show_log();
    }
    this->setWindowTitle(QString::fromStdString("LR1语法分析器GUI"));
}

void MainWindow::show_Action_Goto_tb()
{
    //准备数据
    int wr_num = 0;
    int NT_num = 0;
    istringstream in_wrinfo(get_wr_info(wr_num));
    istringstream in_NTinfo(get_NT_info(NT_num));

    if(this->tb_model_for_AG) delete this->tb_model_for_AG;
    this->tb_model_for_AG = new QStandardItemModel();
    //写好表头
    this->tb_model_for_AG->setHorizontalHeaderItem(0, new QStandardItem("NO."));
    int i;
    for(i = 0; i < wr_num; ++i){
        string wr;
        in_wrinfo >> wr;
        this->tb_model_for_AG->setHorizontalHeaderItem(1+i, new QStandardItem(QString::fromStdString(wr)));
    }
    int j;
    for(j = 0; j < NT_num; ++j){
        string NT;
        in_NTinfo >> NT;
        this->tb_model_for_AG->setHorizontalHeaderItem(1+i+j, new QStandardItem(QString::fromStdString(NT)));
    }
    //tbv关联数据Model
    this->ui->tbv_ActionGoto->setModel(this->tb_model_for_AG);
    //开始写表项
    const int AG_size = get_ActionGoto_size();
    for(int i = 0; i < AG_size; ++i){
        TableIterm tmp = get_ActionGoto(i);
        QList<QStandardItem*> add_items;
        add_items << new QStandardItem(QString::number(i));
        for(int ai = 0; ai < wr_num; ++ai){
            if(tmp.action_map.find(ai) != tmp.action_map.end()){
                std::ostringstream action_str;
                action_str << ((tmp.action_map[ai].action_type == ACTION_T_R)?"r":"s") << tmp.action_map[ai].nx_id;
                printf("action_str test : %s\n", action_str.str().c_str());
                add_items << new QStandardItem(QString::fromStdString(action_str.str()));
            }else{
                add_items << new QStandardItem(QString::fromStdString(" "));
            }
        }
        for(int gi = 0; gi < NT_num; ++gi){
            if(tmp.goto_map.find(10000 + gi) != tmp.goto_map.end()){
               add_items << new QStandardItem(QString::number(tmp.goto_map[10000 + gi]));
            }else{
               add_items << new QStandardItem(QString::fromStdString(" "));
            }
        }
        tb_model_for_AG->appendRow(add_items);
    }
}

void MainWindow::show_Stack()
{
    QFile file(STACK_PATH);
    if(this->tb_model_for_SI) delete this->tb_model_for_SI;
    this->tb_model_for_SI = new QStandardItemModel();
    this->tb_model_for_SI->setHorizontalHeaderItem(0, new QStandardItem("状态栈变化情况"));
    this->ui->tbv_result->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->ui->tbv_result->setModel(this->tb_model_for_SI);

    //--打开文件成功
    if (file.open(QIODevice ::ReadOnly | QIODevice ::Text))
    {
        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            QList<QStandardItem*> add_items;
            add_items << new QStandardItem( textStream.readLine() );
            this->tb_model_for_SI->appendRow(add_items);
        }
    }
    else	//---打开文件失败
    {
        QMessageBox ::information(NULL, NULL, "open file error");
    }
}

void MainWindow::show_Tree()
{
    //执行生成png的命令
    QProcess myProcess(this);
    QString dot_exe = QString(DOT_PATH) + QString(" lr1_tree.dot -T png -o lr1_tree.png");
    //QStringList args;
    //args << ".\\lr1_tree.dot" << "-T" << "PNG" << "-o" << ".\\lr1_tree.png";
    myProcess.start(dot_exe);
    this->ui->lb_tree_img->setText("语法树正在生成中...");
    while(myProcess.waitForFinished(10000) == false){
        qDebug() <<"wait for dot...";
    }
    qDebug() << myProcess.readAllStandardError();
    qDebug() << myProcess.readAllStandardOutput();
    qDebug() << myProcess.errorString();
    qDebug() <<"dot finished!";
    //将png导入QLabel
    this->img=new QImage;
    if(! ( img->load(".\\lr1_tree.png") ) ) //加载图像
    {
        QMessageBox::information(this,
                                 tr("打开图像失败"),
                                 tr("打开图像失败!"));
        delete this->img;
        this->img = NULL;
        return;
    }
    *img = img->scaled(QSize(ui->lb_tree_img->width(), ui->lb_tree_img->height()), Qt::KeepAspectRatio);
    ui->lb_tree_img->setPixmap(QPixmap::fromImage(*img));

}

void MainWindow::show_log()
{

    QFile file(_LOG_PATH);

    //--打开文件成功
    if (file.open(QIODevice ::ReadOnly | QIODevice ::Text))
    {
        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            //---QtextEdit按行显示文件内容
            QString line = textStream.readLine();
            if(line == QString("...由GUI清空...")){
                this->ui->txt_info->clear();
            }else{
                this->ui->txt_info->append(line);
            }

        }
    }
    else	//---打开文件失败
    {
        QMessageBox ::information(NULL, NULL, "日志显示失败");
    }

}

//treeimg 放大
void MainWindow::on_bnt_img_up_clicked()
{
    if(img == NULL)
        return;
    const int owidth = img->width();
    const int oheight = img->height();
    if(! ( img->load(".\\lr1_tree.png") ) ) //加载图像
    {
        return;
    }

    *img = img->scaled(QSize(int(owidth*1.2), int(oheight*1.2)), Qt::KeepAspectRatio);
    ui->lb_tree_img->setPixmap(QPixmap::fromImage(*img));
}
//treeimg 缩小
void MainWindow::on_bnt_img_dw_clicked()
{
    if(img == NULL)
        return;
    const int owidth = img->width();
    const int oheight = img->height();

    *img = img->scaled(QSize(int(owidth*0.8), int(oheight*0.8)), Qt::KeepAspectRatio);
    ui->lb_tree_img->setPixmap(QPixmap::fromImage(*img));

}
//treeimg 适应窗口大小
void MainWindow::on_bnt_img_rst_clicked()
{
    if(img == NULL)
        return;
    if(! ( img->load(".\\lr1_tree.png") ) ) //加载图像
    {
        return;
    }
    *img = img->scaled(QSize(ui->scrollArea->width(), ui->scrollArea->height()), Qt::KeepAspectRatio);
    ui->lb_tree_img->setPixmap(QPixmap::fromImage(*img));
}

void MainWindow::on_bnt_clear_info_clicked()
{

    QFile file(_LOG_PATH);
    if(file.open(QIODevice ::WriteOnly | QIODevice ::Text)){
        file.write("...由GUI清空...\n");
        file.close();
    }else{
        QMessageBox ::information(NULL, NULL, "日志清空失败!");
    }
    show_log();
}

//打开源文件
void MainWindow::on_bnt_showfile_clicked()
{
    if(this->_file_path == "")
        return;

    if(this->readfile_win)
        delete readfile_win;
    this->readfile_win = new ReadFileDialog(this, QString::fromStdString(this->_file_path));
    this->readfile_win->show();
    this->readfile_win->setWindowTitle(QString::fromStdString("源文件内容"));
}

void MainWindow::on_bnt_showlr_clicked()
{
    if(this->_gplst_path == "")
        return;
    if(this->readGplst_win)
        delete readGplst_win;
    this->readGplst_win = new ReadFileDialog(this, QString::fromStdString(this->_gplst_path));
    this->readGplst_win->show();
    this->readGplst_win->setWindowTitle(QString::fromStdString("文法内容"));
}

void MainWindow::on_bnt_showinfo_clicked()
{
    if(this->readlog_win)
        delete readlog_win;
    this->readlog_win = new ReadFileDialog(this, QString::fromStdString(_LOG_PATH));
    this->readlog_win->show();
    this->readlog_win->setWindowTitle(QString::fromStdString("日志文件内容"));
}

void MainWindow::on_bnt_open_infodir_clicked()
{
    QUrl log_url = QUrl::fromLocalFile("./");
    QDesktopServices::openUrl(log_url);
}

void MainWindow::on_bnt_img_open_clicked()
{
    QUrl img_url = QUrl::fromLocalFile(QString::fromStdString(_treepng_path));
    QDesktopServices::openUrl(img_url);
}

// 调用汇编生成器
void MainWindow::on_bnt_start_as_clicked()
{
    this->ui->txt_mips->clear();
    if(!run_as()){
        return;
    }

    // 将mips.asm读取到txt_view中
    QFile file("./mips.asm");
    //--打开文件成功
    if (file.open(QIODevice ::ReadOnly | QIODevice ::Text))
    {
        QTextStream textStream(&file);
        while (!textStream.atEnd()){
            //---QtextEdit按行显示文件内容
            this->ui->txt_mips->append(textStream.readLine());
        }
        file.close();
    }

    // 显示每个代码块的待用活跃信息表
    vector<CodeBlock> cbs = get_codeblocks();
    int cbs_len = cbs.size();
    printf("cbs_len = %d\n", cbs_len);
    for(int i = 0; i < cbs_len; ++i){
        QTextBrowser *cb_page = new QTextBrowser();
        // 加入label
        auto labels = cbs[i].labels;
        for(auto label = labels.begin(); label < labels.end(); ++label){
            QString ss = QString::fromStdString(*label) + QString(":\n");
            cb_page->append(ss);
        }
        // 加入代码
        int code_len = cbs[i].codes.size();
        for(int code_idx = 0; code_idx < code_len; ++code_idx){
            auto code = cbs[i].codes[code_idx];
            char buf[120];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "(%d) (%s, %s, %s, %s)\n", code_idx, code.q1_op.c_str(), code.q2_in1.c_str(), code.q3_in2.c_str(), code.q4_out.c_str());
            stringstream buf_ss;
            buf_ss << buf;
            QString ss = QString::fromStdString(buf_ss.str());
            cb_page->append(ss);
        }
        // 加入待用活跃信息
        QString ss_qua_tb;
        ss_qua_tb = QString::fromStdString(cbs[i].qua_table.print_str());
        QString ss_vua_tb;
        ss_vua_tb = QString::fromStdString(cbs[i].vua_table.print_str());
        cb_page->append(ss_qua_tb);
        cb_page->append(ss_vua_tb);
        this->ui->lst_cb->addItem(QString("块") + QString::number(i));
        int ret = this->ui->stacked_w->addWidget(cb_page);
        printf("gen_page : %d\n", ret);
        fflush(stdout);

    }
    connect(this->ui->lst_cb, SIGNAL(currentRowChanged(int)),this->ui->stacked_w, SLOT(setCurrentIndex(int)));
    show_log();
    this->ui->bnt_start_as->setDisabled(true);
}

void MainWindow::on_bnt_copy_clicked()
{
    QClipboard *clip = QApplication::clipboard();
    clip->clear();
    clip->setText(this->ui->txt_mips->toPlainText());
    QMessageBox::information(this,"成功","已将Mips代码复制到剪贴板",QMessageBox::Ok);
}
