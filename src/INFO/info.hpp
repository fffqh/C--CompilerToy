//实现编译器的信息提示功能
//使用方式；实例化该类为一个全局变量
#ifndef INFO_H
#define INFO_H
#include <string>
#include <fstream>

#define _LOG_PATH "./info.log"

#define _LOG_T_INFO  1
#define _LOG_T_WARN  2
#define _LOG_T_ERROR 3

#define _LOG_STATUS_READY 1
#define _LOG_STATUS_ERROR 2

class INFO{
private:
    std::string _log_path;
    int _status;
    std::ofstream _out;
public:
    INFO(std::string lpath = _LOG_PATH)
    {
        _log_path = lpath;
        _out.open(_log_path, std::ios::out | std::ios::app);
        if(!_out.is_open()){
            _status = _LOG_STATUS_ERROR;
        }else{
            _status = _LOG_STATUS_READY;
        }
    }
    ~INFO(){
        if(_out.is_open()){
            _out.close();
        }
    }
    bool tolog(int type, std::string info)
    {
        if(_status != _LOG_STATUS_READY)
            return false;
        if(type == _LOG_T_INFO){
            _out << "\n[INFO   ] ";
        }else if(type == _LOG_T_WARN){
            _out << "\n[WARNING] ";
        }else if(type == _LOG_T_ERROR){
            _out << "\n[ERROR  ] ";
        }
        _out << info;
        return true;
    }

};

#endif // INFO_H
