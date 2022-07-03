#ifndef CLEX_H
#define CLEX_H
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <regex>
#include <stack>
#include <vector>
#include "../INFO/info.hpp"
using namespace std;

#define CLEX_ERROR -1
#define CLEX_READY 0

struct WInfoItem{
    int index;
    string word;
    string type;
};
struct WRList{
    string type;
    string r;
};
//序号越大，优先级越高
static WRList wr_list[] = {
    {"[标识符]", R"(^[[:alpha:]][[:alnum:]]*)"},
    {"[数值]", R"((([[:digit:]])+(\.){0,1}([[:digit:]]){0,})|(([[:digit:]])+(\.){0,1}([[:digit:]]){0,}))"}, // 1.1 or .1 or 11
    {"[赋值号]", R"([=])"},
    {"[界符]", R"([;])"},
    {"[逗号分割符]", R"([,])"},
    {"[左大括号]", R"([{])"},
    {"[右大括号]", R"([}])"},
    {"[左括号]", R"([(])"},
    {"[右括号]", R"([)])"},
    {"[左中括号]", R"([[])"},
    {"[右中括号]", R"((]))"},
    {"[结束符]", R"([#])"},
    {"[关键字int]", R"((int))"},
    {"[关键字void]", R"((void))"},
    {"[关键字float]", R"((float))"},
    {"[关键字if]", R"((if))"},
    {"[关键字else]", R"((else))"},
    {"[关键字while]", R"((while))"},
    {"[关键字return]", R"((return))"},
    {"[算符+]", R"([+])"},
    {"[算符-]", R"([-])"},
    {"[算符*]", R"([*])"},
    {"[算符/]", R"([/])"},
    {"[算符==]", R"((==))"},
    {"[算符!=]", R"((!=))"},
    {"[算符>]", R"([>])"},
    {"[算符>=]", R"((>=))"},
    {"[算符<]", R"([<])"},
    {"[算符<=]", R"((<=))"},
    {"[结束符#]", R"((#))"},
    {"", R"()"}};
const int HALFBUFSIZE = 128;

class CLEX{
private:
    FILE * _fp;         //文件指针
    FILE * _newfp;      //预处理后的文件指针
    string _fpath;      //原始文件路径
    string _newfpath;   //预处理后的文件路径
    int  _status;       //词法分析器的当前状态
    char _buf[2*HALFBUFSIZE];   //缓冲区
    int  _pst;                  //指向缓冲的指针（index）
    int  _ped;                  //指向缓冲的指针（index）

    WRList * _wr_list;    //指向定义的词法表(type + regex expression)
    
    
    bool update_buf(int half_index)
    {
        if(feof(_newfp)){
            //printf("[error]已达文件尾，继续读入失败！\n");
            return false;
        }
        memset(_buf + half_index * HALFBUFSIZE, 0, HALFBUFSIZE);
        //printf("test fread ret = %d\n", 
                (int)fread(_buf + half_index * HALFBUFSIZE, HALFBUFSIZE, 1, _newfp);
        //)
        
        return true;
    }

public:
    enum { MAX_TYPE_BUFSIZE = 20, MAX_R_BUFSIZE = 256};
    CLEX(string file_path)
    {
        memset(_buf, 0, 2*HALFBUFSIZE);
        _pst = 0;
        _ped = 0;
        _newfp = NULL;
        _fpath = file_path;
        _fp = fopen(file_path.c_str(), "r");
        _wr_list = wr_list;
        INFO clex_info;
        ostringstream out;
        if(!_fp){
            //tolog
            out << "clex : 文件(" << file_path << ")打开失败...";
            clex_info.tolog(_LOG_T_ERROR, out.str());
            _status = CLEX_ERROR; //出错
        }else{
            clex_info.tolog(_LOG_T_INFO, "clex : 词法分析器正常启动.");            
            _status = CLEX_READY; //就绪
        }
    }
    ~CLEX()
    {
        // if(_fp) fclose(_fp);
        if(_newfp) fclose(_newfp);
        if(_wr_list!=wr_list) delete[]_wr_list;
    }
    WRList * get_wr_list() const
    {
        return _wr_list;
    }

    /** 设置词法分析器 **/
    bool wr_list_tofile(string fpath) const
    {   //将当前的词法定义信息写入文件
        FILE * fp = fopen(fpath.c_str(), "w");
        if(!fp){
            printf("[error]创建导出词法定义文件失败(fpath = %s)\n", fpath.c_str());
            return false;
        }
        for(int i = 0; _wr_list[i].type != ""; ++i){
            char tmp_type[MAX_TYPE_BUFSIZE] = {0};
            memcpy(tmp_type, _wr_list[i].type.c_str() + 1, _wr_list[i].type.length()-2);
            fprintf(fp, "%s  %s\n", tmp_type, _wr_list[i].r.c_str());
        }
        fclose(fp);
        return true;
    }
    bool set_wrlist(string fpath)
    {
        //读取文件中的词法定义信息，设置词法分析器
        FILE *fp = fopen(fpath.c_str(), "r");
        INFO clex_info;
        ostringstream out;
        if(!fp){
            out << "clex : 读取词法正则定义文件失败(fpath = " << fpath << ").";
            clex_info.tolog(_LOG_T_ERROR, out.str());
            return false;
        }
        vector<WRList> tmp_wrlst;
        while(1){
            WRList tmp;
            char type_buf[MAX_TYPE_BUFSIZE];
            char r_buf[MAX_R_BUFSIZE];
            if(fscanf(fp, " %s %s", type_buf, r_buf)==EOF)
                break;
            type_buf[MAX_TYPE_BUFSIZE-1] = '\0';
            r_buf[MAX_R_BUFSIZE-1] = '\0';
            tmp.type = type_buf;
            tmp.r    = r_buf;
            tmp_wrlst.push_back(tmp);
        }
        //释放老词表的空间（除默认情况）
        if(_wr_list != wr_list) 
            delete[]_wr_list;
        //为新词表动态分配空间
        _wr_list = new (nothrow) WRList[tmp_wrlst.size() + 1];
        if(!_wr_list){
            clex_info.tolog(_LOG_T_ERROR, "clex : 更新词表动态申请空间失败");
            _status = CLEX_ERROR;
            return false;
        }
        unsigned int i = 0;
        for(i = 0; i < tmp_wrlst.size(); ++i){
            _wr_list[i].type = "[" + tmp_wrlst[i].type + "]";
            _wr_list[i].r = tmp_wrlst[i].r;
        }
        _wr_list[i].type = "";
        _wr_list[i].r = "";
        out << "clex : 更新词表成功(fpath = " << fpath << ").";
        clex_info.tolog(_LOG_T_INFO, out.str());
        return true;
    }

    /** 调用词法分析器 **/
    bool predo()
    {
        INFO clex_info;
        ostringstream out;
        //预处理：1、去除换行符 2、多个空白符合并为一个
        _newfp = NULL;
        _newfpath = string(_fpath + ".predo");
        _newfp = fopen(_newfpath.c_str(), "w");
        if(!_newfp){
            out << "clex : 新建预处理文件(" << _newfpath << ")失败...";
            clex_info.tolog(_LOG_T_ERROR, out.str());
            //printf("[error]新建预处理文件(%s)失败...\n", _newfpath.c_str());
            _status = CLEX_ERROR;
            return false;
        }
        char ch;
        bool blank = 0;
        while(1){
            ch = fgetc(_fp);
            if(feof(_fp))
                break;
            if(ch == '\n' || ch == '\t')
                ch = ' ';
            if((ch == ' ' || ch == '\t') && blank){
                continue;
            }else{
                blank = 0;
            }
            //将ch写入_newfp;
            fputc(ch, _newfp);
            if(ch == ' ' || ch == '\t')
                blank = 1;
        }
        fclose(_fp);
        fclose(_newfp);
        _newfp = fopen(_newfpath.c_str(), "r");
        
        //装入_buf
        update_buf(0);
        update_buf(1);

        clex_info.tolog(_LOG_T_INFO, "clex : 预处理文件生成成功.");
        return true;
    }
    void pstatus()
    {
        if(_status == CLEX_READY)
            printf("[status] CLEX_READY\n");
        else if(_status == CLEX_ERROR)
            printf("[status] CLEX_ERROR\n");
    }
    int  get_status()const
    {
        return _status;
    }
    struct WInfoItem getword()
    {
        INFO clex_info;
        //0、检查词法分析器当前状态
        if(_status != CLEX_READY){
            clex_info.tolog(_LOG_T_ERROR, "clex : getword失败，由于CLEX状态错.");
            return {-1,"",""};
        }
        //1、将 buf 中的数据按顺序存放入 ubbuf (unblock buf)
        char ubbuf[2*HALFBUFSIZE + 1] = {0};
        strncpy(ubbuf, _buf + _pst, 2*HALFBUFSIZE - _pst);
        if(_pst >= HALFBUFSIZE){
            strncpy(ubbuf + 2*HALFBUFSIZE - _pst, _buf, HALFBUFSIZE);
        }

        //3、检查buf是否为空
        if(!strlen(ubbuf)){
            return {-1,"",""};
        }
        
        //用于存结果的结构体
        struct {
            int len;
            int index;
            string match_str;
            bool ispostblank;
        } rlt;
        rlt.len = 0;
        rlt.index = -1;
        rlt.match_str = "";

        //4、开始按长度增长匹配：
        //字符串长度增长匹配，越后面的匹配优先级越高（满足最大长度优先、正规式优先级策略）
        for(int i = 0; ubbuf[i] != 0; ++i){
            char ch = ubbuf[i + 1];
            ubbuf[ i + 1 ] = 0;
            int ifmatch = false;
            for(int j = 0; _wr_list[j].type != ""; ++j){
                regex re(_wr_list[j].r);
                if(regex_match(ubbuf, re)){
                    rlt.len = i + 1;
                    rlt.index = j;
                    rlt.match_str = ubbuf;
                    rlt.ispostblank = (ch == ' ');
                    ifmatch = true;
                    // printf("test:一次匹配成功(%s)\n", rlt.match_str.c_str());
                }
            }
            ubbuf[i + 1] = ch;
            if(!ifmatch){
                break;
            }
        }
        //5、检查是否匹配失败
        if(!(rlt.len)){
            ostringstream out;
            out << "clex : 词法分析语法错误！(附近：" << ubbuf << ").";
            clex_info.tolog(_LOG_T_ERROR, out.str());
            // printf("[error]语法错误!(附近：%s)\n", ubbuf);
            return {-1,"",""};
        }
        //6、匹配成功，开始处理匹配：1、构造返回值 2、移动指针，调整缓冲区
        ostringstream out;
        out << "<" << rlt.index << "," << rlt.match_str <<","<< _wr_list[rlt.index].type << ">";
        _ped = _pst + rlt.len + !!(rlt.ispostblank);
        if(_pst < HALFBUFSIZE && _ped >= HALFBUFSIZE){
            //前半缓冲区刷新
            update_buf(0);
        }
        else if(_pst < 2*HALFBUFSIZE && _ped >= 2*HALFBUFSIZE){
            //后半缓冲区刷新
            update_buf(1);
            _ped %= 2*HALFBUFSIZE;
        }
        _pst = _ped;
        return {rlt.index, rlt.match_str, _wr_list[rlt.index].type};
    }
};

#endif // CLEX_H
