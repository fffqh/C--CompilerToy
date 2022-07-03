#include "../CLEX/clex.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <set>
#include <map>

//非终结符
struct NToken {
    std::string name;
    std::string desc;
};

//符号表类
class TokenManager {
public:
    NToken * NT_list; //非终结符表
    WRList * wr_list; //终结符表
    TokenManager()
    {
        NT_list = NULL;
        wr_list = NULL;
    }
    TokenManager(NToken* NTl, WRList * wrl)
    {
        NT_list = NTl; 
        wr_list = wrl; 
    }
    int get_NT_vindex(std::string NT_name) const 
    {
        for(int i = 0; NT_list[i].name != ""; ++i){
            if(NT_list[i].name == NT_name){
                return 10000 + i; 
            }
        }
        return -1;
    }
    int get_wr_vindex(std::string wr_name) const
    {
        for(int i = 0; wr_list[i].type!= ""; ++i){
            if(wr_list[i].type == wr_name)
                return i;
        }
        return -1;
    }
};

//文法产生式 Grammar Prodection
struct GP {
    int                 LHS; //产生式左部的符号虚索引
    std::vector<int>    RHS; //产生式右部的符号虚索引序列
};

class GPlistManager {
public:
    std::vector<GP> GP_list;           //文法产生式集合
    std::set<int>   NT_epsilon_vidset; //空产生式集合
    GPlistManager()
    {
        GP_list.clear();
        NT_epsilon_vidset.clear();
    }
    bool ini(std::string fpath, TokenManager* all_token)
    {
        GP_list.clear();
        NT_epsilon_vidset.clear();
        ifstream fin;
        fin.open(fpath, ios::in);
        if(!fin.is_open()) {
            //tolog
            return false;
        }
        //读取文件中的每行数据（允许空行）
        while(1){
            std::string tmp = "";
            getline(fin, tmp);
            if(tmp != ""){ //** 处理一个文法产生式
                tmp += string(" ");
                istringstream gp_in(tmp);

                //1、处理产生式左部
                std::string LHS_string;
                gp_in >> LHS_string;
                int LHS_vindex = all_token->get_NT_vindex(LHS_string);
                if(LHS_vindex == -1){
                    fin.close();
                    //tolog
                    return false;
                }
                //2、处理产生式右部
                std::vector<int> RHS_vindex;
                RHS_vindex.clear();
                while(1){
                    string RHS;
                    gp_in >> RHS;
                    if(gp_in.eof()) break; //到达行末，退出。
                    int wr_vindex = all_token->get_wr_vindex(RHS);                    
                    if(wr_vindex != -1){
                        RHS_vindex.push_back(wr_vindex);
                        continue;
                    }
                    int NT_vindex = all_token->get_NT_vindex(RHS);
                    if(NT_vindex != -1){
                        RHS_vindex.push_back(NT_vindex);
                        continue;
                    }
                    fin.close();
                    //tolog : 非法的右部符号
                    return false;
                }

                if(RHS_vindex.size() < 1){
                    this->NT_epsilon_vidset.insert(LHS_vindex);
                    ; //空产生式
                }

                //3、构造产生式，并存入产生式表中
                GP myGP = {LHS_vindex, RHS_vindex};
                this->GP_list.push_back(myGP);
            
            }
            if(fin.eof())
                break;
        }
        fin.close();
        //tolog : info 构造文法产生式完毕！(共n个文法产生式)
        return true;
    }
    void debug_print() const
    {
        for(unsigned int i = 0; i < GP_list.size(); ++i){
            printf("%d : ", GP_list[i].LHS);
            printf("[");
            for(unsigned int j = 0; j < GP_list[i].RHS.size(); ++j){
                printf("%d ", GP_list[i].RHS[j]);
            }
            printf("]\n");
        }
    }
    //求一个非终结符的 First 集，返回一个std::set<int>，由终结符的 vindex 组成
    //递归方式:) & 不存在epsilon产生式
    //trick 一下 : 特判右部为空的非终结符，first_set 为空，且不考虑它。
    std::set<int> get_NT_FIRST(int NT_vindex, int newget = 1) const
    {
        if(NT_epsilon_vidset.find(NT_vindex) != NT_epsilon_vidset.end())
            return std::set<int>();
        
        static std::set<int> first_set;
        static std::set<int> done_vindex;
        if(newget){
            first_set.clear();
            done_vindex.clear();
        }
        for(std::vector<GP>::const_iterator gp_iter = GP_list.begin(); gp_iter != GP_list.end(); ++gp_iter){
            if(gp_iter->LHS == NT_vindex && gp_iter->RHS.size()){
                int first_vid = -1;
                for(auto iter = gp_iter->RHS.begin(); iter != gp_iter->RHS.end(); ++iter){
                    if(NT_epsilon_vidset.find(*iter)==NT_epsilon_vidset.end()){
                        first_vid = *iter;
                        break;
                    }
                }
                if(first_vid == -1) continue;
                
                if(first_vid / 10000){  //非终结符
                    if( (NT_vindex != first_vid) && //左递归则跳过
                        (done_vindex.find(first_vid)==done_vindex.end())){ //该非终结符未处理过
                        get_NT_FIRST(first_vid, 0);
                        done_vindex.insert(first_vid);
                    }
                }else{                  //终结符
                    if(first_set.find(first_vid)==first_set.end()){ //该终结符未加入
                        first_set.insert(first_vid);
                    }
                }
            }
        }
        return first_set;
    }
};

/* about: 符号虚索引 & 符号实索引

    - 虚索引通过固定规则映射至实索引
    - 1、终结符: 实索引=虚索引
    - 2、非终结符：实索引=虚索引%10000 （上限为9999）
    - 通过虚索引可以判断该符号的类型： vindex/10000 为真 -> 非终结符

-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// 属性符号
/*  about: 带有属性的符号
    由于后续工作未知，为了提高可扩展性，将其定义如下

    about: 属性
    -- ---- ---------
    0  name  string
    -- ---- ---------
    1  type  vtype
    -- ---- ---------
    2  offset unsigned
    -- ---- ---------
    3  value type_base
    -- ---- ---------
    4  place string
    -- ---- ---------
    5  op    string
    -- ---- ---------
    6  tlst  set<int>
    -- ---- ---------
    7  flst  set<int>
    -- ---- ---------
    8  nlst  set<int>
    -- ---- ---------
    9  nidx  int
    -- ---- ---------
*/
enum vtype{type_null, type_int, type_float, type_void};
struct ftype{
    vtype ret_type;
    std::vector<vtype> par_type;
};
enum symbol_ptype{sym_name,sym_type,sym_offset,sym_value,sym_place,sym_op,sym_tlst,sym_flst,sym_nlst,sym_nidx};
class Symbol {
public:
    int token_idx; //对应符号的虚索引 
    //std::map<int, void*> pmap; //属性指针的pmap (int:ptype  void*:属性) 
private:
    string   sym_name;
    vtype    sym_type;
    int      sym_value_int;
    string   sym_place;
    string   sym_op;
    set<int> sym_tlst;
    set<int> sym_flst;
    set<int> sym_nlst;
    int      sym_nidx;

public:
    Symbol()
    {
        this->sym_name = "";
        this->sym_type = vtype::type_null;
        this->sym_value_int = 0;
        this->sym_place = "";
        this->sym_op = "";
        this->sym_tlst = set<int>();
        this->sym_flst = set<int>();
        this->sym_nlst = set<int>();
        this->sym_nidx = 0;
    }

    // 符号属性操作接口
    bool   add_name(string name)
    {
        this->sym_name = name;
        return true;
    }
    string get_name()
    {
        return this->sym_name;
    }
    bool   add_type(vtype t)
    {
        this->sym_type = t;
        return true;
    }
    vtype  get_type()
    {
        return this->sym_type;
    }
    bool   add_value_int(int v)
    {
        // 检查该符号目前的类型限定情况
        if(this->sym_type != vtype::type_int)
            return false;
        // 将value_int赋值给该符号
        this->sym_value_int = v;
        return true;
    }
    int    get_value_int()
    {
        // 检查该符号是否为 int 类型
        if(this->sym_type != vtype::type_int){
            printf("[ERROR]get_value_int 符号出现类型错误..\n");
            return 0;
        }
        return this->sym_value_int;
    }
    
    bool   add_place(string place)
    {
        this->sym_place = place;
        return true;
    }
    string get_palce()
    {
        return this->sym_place;
    }
    bool   add_op(string op)
    {
        this->sym_op = op;
        return true;
    }
    string get_op()
    {
        return this->sym_op;
    }
    bool   mk_tlst() //有则返回，无则创建
    {
        return true;
    }
    bool   up_tlst(int code_idx)
    {
        auto insert_rlt = this->sym_tlst.insert(code_idx);
        return insert_rlt.second; //返回是否插入成功
    }
    set<int> get_tlst()
    {
        return this->sym_tlst;
    }
    bool   mk_flst() //有则返回，无则创建
    {
        return true;
    }
    bool   up_flst(int code_idx)
    {
        auto insert_rlt = this->sym_flst.insert(code_idx);
        return insert_rlt.second; //返回是否插入成功
    }
    set<int> get_flst()
    {
        return this->sym_flst;
    }
    bool   mk_nlst() //有则返回，无则创建
    {
        return true;
    }
    bool   up_nlst(int code_idx)
    {
        auto insert_rlt = this->sym_nlst.insert(code_idx);
        return insert_rlt.second; //返回是否插入成功
    }
    set<int> get_nlst()
    {
        return this->sym_nlst;
    }
    bool  add_nidx(int next_idx)
    {
        this->sym_nidx = next_idx;
        return true;
    }
    int   get_nidx()
    {
        return this->sym_nidx;
    }
};

// 内存符号表
struct SymTableItem{
    unsigned int offset;
    enum     vtype type;
};
struct FuncNameItem{
    unsigned int offset;
    ftype    type;
    int tbidx;
};
class MemSymTable{
private:
    unsigned int offset_count;
public:
    std::vector<std::map<string, SymTableItem>> tbs; //每个函数域的符号表: name, info 
    //std::map<string, SymTableItem> tb; //符号表：name, info
    std::map<string, FuncNameItem> ftb; //函数名表：function name, info
    int cur_tbidx; // 当前内存符号管理器中活跃的函数域符号表的index
    MemSymTable()
    {
        offset_count = 0;
    }
    //新增一个函数域符号表
    int add_tb()
    {
        tbs.push_back(std::map<string, SymTableItem>());
        return tbs.size()-1;
    }
    //给符号表添加符号项
    bool add_item(string name, enum vtype t, int tbidx = -1)
    {
        if(tbidx == -1)
            tbidx = cur_tbidx;
        tbs[tbidx][name] = {offset_count++, t};
        return true;
    }
    bool find_item(string name, int tbidx = -1)
    {
        if(tbidx == -1) 
            tbidx = cur_tbidx;
        auto find_it = tbs[tbidx].find(name);
        if(find_it == tbs[tbidx].end())
            return false;
        return true;
    }
    unsigned int get_offset(string name, int tbidx = -1)
    {
        if(tbidx == -1)
            tbidx = cur_tbidx;
        return tbs[tbidx][name].offset;
    }
    vtype get_type(string name, int tbidx = -1)
    {
        if(tbidx == -1)
            tbidx = cur_tbidx;
        return tbs[tbidx][name].type;
    }

    //给函数表添加符号项
    bool add_fitem(string fname, unsigned int offset, ftype t, int tbidx)
    {
        ftb[fname] = {offset, t, tbidx};
        return true;
    }
    bool find_fitem(string fname)
    {
        auto find_fit = ftb.find(fname);
        if(find_fit == ftb.end())
            return false;
        return true;
    }
    unsigned int get_offset_func(string fname)
    {
        return ftb[fname].offset;
    }
    ftype get_ftype_func(string fname)
    {
        return ftb[fname].type;
    }
    int get_tbidx_func(string fname)
    {
        return ftb[fname].tbidx;
    }
    bool table_to_file(fstream & fout)
    {
        int num = 0;
        for(auto iter_tb = tbs.begin(); iter_tb < tbs.end(); ++iter_tb)
            for(auto iter_v = iter_tb->begin(); iter_v != iter_tb->end(); ++iter_v)
                num += 1;
        fout << num << "\n";
        fout << "* Symbol Table -----------------\n";
        fout << "  tbidx | name | offset | type  \n";
        fout << "  ------------------------------\n";
        int tbs_len = tbs.size();
        for(int i = 0; i < tbs_len; ++i){
            for(auto iter = tbs[i].begin(); iter != tbs[i].end(); ++iter){
                fout<< "  " << i 
                    << " "  << iter->first 
                    << " "  << iter->second.offset 
                    << " "  << iter->second.type 
                    << "\n";
            }
        }
        fout << "  ------------------------------" << endl;

        fout << "* Function Table ----------------\n";
        fout << "  fname | offset | ftype | tbidx \n";
        fout << " --------------------------------\n";
        for(auto iter = ftb.begin(); iter != ftb.end(); ++iter){
            fout<< "  " << iter->first << " "  << iter->second.offset << " "; 
            for(auto t = iter->second.type.par_type.begin(); t != iter->second.type.par_type.end(); ++t){
                fout << *t << ",";
            }
            fout<< "->" << iter->second.type.ret_type << " ";
            fout<< iter->second.tbidx << "\n";
        }
        fout << " ---------------------------------" << endl;
        return true;
    }

};
// 形参栈使用的一个形参记录类型，用于函数声明翻译
struct FormParam{
    std::string name; //形参的名称
    enum vtype type;  //形参的类型
};
// 形参表使用的一个形参记录类型，用于函数声明翻译
struct FormParamTableItem{
    int index; //形参在形参表中的索引
    enum vtype type; //形参的类型
};
// 规范 LR1 项目
class LR1_ITERM {
public:
    int  GPi;                   //文法产生式的索引
    int  dot;                   //点所在的位置
    std::set<int> fw1_word;     //向前看1位的符号集
    bool operator==(const LR1_ITERM & t) const
    {
        if((this->GPi == t.GPi) && (this->dot == t.dot)
            && (this->fw1_word == t.fw1_word)){
            return true;
        }
        return false;
    }
    bool operator<(const LR1_ITERM & t) const
    {
        if(*this == t){
            return false;
        }
        if(this->GPi > t.GPi)
            return false;
        if((this->GPi == t.GPi) && (this->dot > t.dot))
            return false;
        if((this->GPi == t.GPi) && (this->dot == t.dot) 
            && this->fw1_word > t.fw1_word)
            return false;
        if((this->GPi == t.GPi) && (this->dot == t.dot) 
            && this->fw1_word == t.fw1_word )
            return false;
        return true;
    }
    
    //判断此lr1项目的可归约性
    bool is_r(GPlistManager* GPm) const
    {//dot是否位于GP的末尾
        return 
        (int)(GPm->GP_list[this->GPi].RHS.size()) == this->dot;
    }

};

//状态：规范 LR1 项目集对应
class ITERMS {
public:
    std::map<LR1_ITERM, bool> iterms;

    bool operator==(const ITERMS & t) const 
    {
        if(this->iterms == t.iterms)
            return true;
        return false;
    }
    bool operator<(const ITERMS & t) const
    {
        if(this->iterms < t.iterms)
            return true;
        return false;
    }
};

struct ITERMS_STATE{
    bool isclose;
    int  index;
};

// LR 语法分析表表项
struct Action {
    char action_type;   //移入 or 归约 or 接受
    int  nx_id;         //下一个状态编号
};
struct TableIterm {
    std::map<int, Action> action_map; //wr_vid, Action
    std::map<int, int> goto_map;      //nt_vid, next_state_id
};
#define ACTION_T_S 1
#define ACTION_T_R 2
//tips: 用 r0 表示接受
#define STACK_PATH "./stack_info.txt"

// 四元式生成器与语义子程序
struct FOUR_CODE_ITEM{
    int    idx;
    string  op;
    string  s1;
    string  s2;
    string   d;
};
class IL {
public:
    map<int, FOUR_CODE_ITEM> _code_buf;
    map<int, FOUR_CODE_ITEM> _ready_buf; //候选控制语句集合，待回填，或待抛弃
    int idx_next; // 下一条语句的地址
    int idx_temp; // 临时变量的编号
    IL() { 
        idx_next = 1; 
        idx_temp = 0;
    }

    int  gen(string op, string s1, string s2, const string d)
    {
        int code_idx = idx_next++;
        FOUR_CODE_ITEM gen_code = {code_idx, op, s1, s2, d};
        _code_buf[code_idx] = gen_code;
        return code_idx;
    }
    int  gen_ready(string op, string s1, string s2, const string d)
    {
        int code_idx = idx_next++;
        FOUR_CODE_ITEM gen_code = {code_idx, op, s1, s2, d};
        _ready_buf[code_idx] = gen_code;
        return code_idx;
    }
    // 回填第 4 域，并将该语句从 ready_buf 移入 code_buf
    bool back_patch(int code_idx, const string value)
    {
        auto find_ready = _ready_buf.find(code_idx);
        if(find_ready == _ready_buf.end()){
            printf("[back_patch error] 回填目标代码不存在...\n");
            return false;
        }
        //1、取得四元式
        FOUR_CODE_ITEM item = find_ready->second;
        //2、回填
        item.d = value;
        //3、加入 _code_buf
        _code_buf[code_idx] = item;
        //4、移出 _ready_buf
        _ready_buf.erase(code_idx);
        //printf("debug: finish back_patch code_idx=%d, value=%s\n", code_idx, value.c_str());
        return true;
    }
    bool code_to_file(fstream& fout)
    {
        int max_idx = 0;
        for(auto iter = _code_buf.begin(); iter != _code_buf.end(); ++iter){
            fout        << (iter->first) << 
                    "(" << (iter->second.op) << "," 
                        << (iter->second.s1) << "," 
                        << (iter->second.s2) << "," 
                        <<(iter->second.d)<< ");\n";
            if(max_idx < iter->first) max_idx = iter->first;
        }
        map<int, FOUR_CODE_ITEM>().swap(_code_buf); // 清空 
        
        fout <<"------------dropped-------------\n";
        for(auto iter = _ready_buf.begin(); iter != _ready_buf.end(); ++iter){
            fout        << (iter->first) << 
                    "(" << (iter->second.op) << "," 
                        << (iter->second.s1) << "," 
                        << (iter->second.s2) << "," 
                        <<(iter->second.d)<< ");\n";
            if(max_idx < iter->first) max_idx = iter->first;
        }
        map<int, FOUR_CODE_ITEM>().swap(_ready_buf); // 清空 

        return true;
    }
    int  get_idx_next() const
    {
        return idx_next;
    }
    string newtemp()
    {
        ostringstream out;
        out << "T" << idx_temp++;
        return out.str();
    }
    void reset_temp()
    {
        idx_temp = 0;
    }
};

// LR1 非static函数的声明
// bool lalr_ini();

bool lalr_run(string file_path, string gplst_fpath);

string get_token_lst();

string get_GP_lst();

string get_wr_info(int & wr_num);

string get_NT_info(int & NT_num);

TableIterm get_ActionGoto(int TI_index);

const int get_ActionGoto_size();
