#ifndef TOMIPS_H
#define TOMIPS_H
#include<stdio.h>
#include<string>
#include<vector>
#include<queue>
#include<map>
using namespace std;

// DAG的结点
class DAGNode{
public:
    string value;
    string op;
    vector<string> left_sym;
};
class DAGManager{
public:
    vector<DAGNode> node_list;
    map<string, int> sym_node_map; // sym_string, node_index
};

// 四元式
class FOUR_CODE{
public:
    string q1_op;
    string q2_in1;
    string q3_in2;
    string q4_out;
};

// 待用活跃信息
struct ua{
    int use;
    bool active;
};

// 四元式待用活跃信息表
class Q_UA_TABLE{
public:
    vector<ua> out_ua_tb;
    vector<ua> in1_ua_tb;
    vector<ua> in2_ua_tb;
    int table_size;
    void init(int size)
    {
        ua tmp_ua = {-1, false};
        for (int i = 0; i < size; ++i)
        {
            this->out_ua_tb.push_back(tmp_ua);
            this->in1_ua_tb.push_back(tmp_ua);
            this->in2_ua_tb.push_back(tmp_ua);
        }
        table_size = size;
    }
    void print()
    {
        printf("\n\tqid | out , in1, in2 \n---------------------------\n");
        for(int i = 0; i < table_size; ++i){
            printf("\t%d\t (%d,%d)\t(%d,%d)\t(%d,%d)\n",
                i, out_ua_tb[i].use, out_ua_tb[i].active,
                in1_ua_tb[i].use, in1_ua_tb[i].active,
                in2_ua_tb[i].use, in2_ua_tb[i].active);
        }
        printf("---------------------------\n");
    }
    string print_str()
    {
        stringstream ss;
        ss << "\n  qid | out, in1, int \n------------------------------------\n";
        for (int i = 0; i < table_size; ++i)
        {
            char buf[120];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, " %d (%d,%d) (%d,%d) (%d,%d)\n",
                    i, out_ua_tb[i].use, out_ua_tb[i].active,
                    in1_ua_tb[i].use, in1_ua_tb[i].active,
                    in2_ua_tb[i].use, in2_ua_tb[i].active);
            ss << buf;
        }
        ss << "------------------------------------\n";
        return ss.str();
    }
};
// 符号表待用活跃信息
class V_UA_TABLE{
public:
    map<string, ua> v_ua_tb;
    void print()
    {
        printf("\n\tvname | use, active \n---------------------------\n");
        for(auto iter = v_ua_tb.begin(); iter != v_ua_tb.end(); ++iter){
            printf("\t%s\t(%d,%d)\n", iter->first.c_str(), iter->second.use, iter->second.active);
        }
        printf("---------------------------\n");
    }
    string print_str()
    {
        stringstream ss;
        ss << "\n  vname | use, active \n------------------------------------\n";
        for(auto iter = v_ua_tb.begin(); iter != v_ua_tb.end(); ++iter){
            char buf[120];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, " %s\t(%d,%d)\n", iter->first.c_str(), iter->second.use, iter->second.active);
            ss << buf;
        }
        ss << "------------------------------------\n";
        return ss.str();
    }
};

// 代码块
class CodeBlock{
public:
    DAGManager dag_manager;
    vector<string> labels;  // 该代码块的标签
    set<string> out_active; // 出口活跃变量
    vector<int> nbs_idx;    // 代码块流图的下一个
    vector<FOUR_CODE> codes; // 代码块中的代码
    Q_UA_TABLE qua_table;    // 四元式的待用活跃信息
    V_UA_TABLE vua_table;    // 符号表的待用活跃信息
    int st_code_index;
    int ed_code_index;
};

// 读取代码时的缓存结构
class INPUT_CODE{
public:
    int index;
    FOUR_CODE code;
};


// 所有可用的寄存器名称
const static char *reg_names[] = {
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
    NULL
};

// 寄存器资源
struct Register{
    string value;
    bool   is_empty;
};
// 所有可用的临时寄存器资源
class REGS{
public:
    map<string, Register> regs_map; 
    void init()
    {
        for(int i = 0; reg_names[i]!=NULL; ++i){
            Register r = {"", true};
            regs_map[reg_names[i]] = r;
        }
    }
    string find_empty_reg()
    {
        string reg_name = "";
        for(auto reg = regs_map.begin(); reg != regs_map.end(); ++reg){
            if(reg->second.is_empty)
                reg_name = reg->first;
        }
        return reg_name;
    }
};

// AVALUE 
struct AddrV{
    bool is_reg;
    string addr;
};

bool run_as();
vector<CodeBlock> get_codeblocks();

#endif // TOMIPS_H
