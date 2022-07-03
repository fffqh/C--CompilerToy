#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <queue>
#include <set>
#include"toMips.h"
#include "../INFO/info.hpp"
using namespace std;

#define OP_j "j"            
#define OP_jnz "jnz"        
#define OP_fuzhi "="        
#define OP_jeq "jeq"
#define OP_jne "jne"
#define OP_jl "j<"
#define OP_jle "j<="
#define OP_jg "j>"
#define OP_jge "j>="
#define OP_add "+"
#define OP_sub "-"
#define OP_mul "*"
#define OP_dev "/"
#define OP_param "param"
#define OP_call "call"
#define OP_ret "ret"

// 符号的映射关系

// 读取文件时，记录的起始行号与结束行号
static set<int> start_index;
static set<int> end_index;
// 记录函数的参数数量
static map<string, int> func_map;
// 记录内存变量的数量
static int mem_param_num;
// 从文件中读取的代码缓存
static queue<INPUT_CODE> input_code_buf;

// 标签的map
// 1. 在划分块之前，记录标签所对应的代码行号
// 2. 在划分块时，每次处理完标签对应的行，将map值改为块号
// 3. 在划分块时，在处理块中转移语句的目标标签时，若标签map值还是行号，则以负数的方式记录标签号至nbs_idx中
static map<string, int> label_map;

// 所有代码块
static vector<CodeBlock> code_blocks;

static REGS RVAULE;
static map<string, vector<AddrV>> AVALUE;

// 从ilcode文件中读取四元式代码，并进行代码块划分
bool inputcode(string ilcode_file_path)
{
    FILE* fp = NULL;
    fp = fopen(ilcode_file_path.c_str(), "r");
    if(!fp){
        printf("[ERROR]中间代码文件打开失败(%s)\n", ilcode_file_path.c_str());
        return false;
    }
    char buf[256];
    // 读取首行，得到mem_param_num
    memset(buf,0,sizeof(buf));
    fgets(buf, sizeof(buf), fp);
    stringstream ss;
    ss << buf;
    ss >> mem_param_num;
    // 读取Function Table中的Label信息
    int  count_xinhao = 0;
    while(1){
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        if(buf && buf[0]=='*')
            count_xinhao += 1;
        if(count_xinhao == 2)
            break;
        if(!buf || feof(fp)){
            printf("[ERROR]中间代码文件符号表格式不正确(%s)\n", ilcode_file_path.c_str());
            fclose(fp);
            return false;
        }
    }
    printf("开始读取符号表\n");
    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);
    // 开始读取符号表
    while (1){
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        if(!buf || feof(fp)){
            printf("[ERROR]中间代码文件符号表格式不正确(%s)\n", ilcode_file_path.c_str());
            fclose(fp);
            return false;
        }
        // 检查是不是最后一行
        if(buf[1]=='-' && buf[2]=='-'){
            break;
        }
        // 对有效的行数据进行处理
        stringstream ss;
        string fname;
        int foffset;
        ss << buf;
        ss >> fname >> foffset;
        label_map[fname] = foffset;
        // 读取函数的参数，确定函数参数的个数
        int param_num = 0;
        while(true){
            char tmp;
            ss >> tmp;
            if(tmp == ','){
                ss >> tmp;
            }
            if(tmp == '-'){
                break;
            }
            param_num += 1;
        }
        func_map[fname] = param_num;
    }
    printf("符号表读取结束\n");
    // 将所有函数开始的第一个四元式加入start
    for(auto iter = label_map.begin(); iter!=label_map.end(); ++iter){
        start_index.insert(iter->second);
    }
    start_index.insert(1);
    // 开始读取四元式
    int counter = 0;
    while(1){
        int code_idx;
        string q1_op;
        string q2_in1;
        string q3_in2;
        string q4_out;

        memset(buf, 0, sizeof(buf));

        fgets(buf, sizeof(buf), fp);
        if(!buf || buf[1]=='-'){
            printf("[INFO]共读取四元式 %d 条.\n", counter);
            break;
        }
        for(int i = 0; i < sizeof(buf); ++i){
            if(buf[i]=='(' || buf[i]==')' || buf[i]==',')
                buf[i] = ' ';
            if(buf[i] == 0)
                break;
        }

        stringstream ss;
        ss << buf;
        ss >> code_idx >> q1_op >> q2_in1 >> q3_in2 >> q4_out;
        //printf("解析四元式：%d %s %s %s %s\n", code_idx, q1_op.c_str(), q2_in1.c_str(), q3_in2.c_str(), q4_out.c_str());

        // 解析四元式
        if (q1_op == OP_j   ||
            q1_op == OP_jnz ||
            q1_op == OP_jeq ||
            q1_op == OP_jne ||
            q1_op == OP_jl  ||
            q1_op == OP_jle ||
            q1_op == OP_jg  ||
            q1_op == OP_jge){

            label_map[string("L") + q4_out] = atoi(q4_out.c_str());
            // 转移语句转到的语句加入 start
            start_index.insert(atoi(q4_out.c_str()));
            // 转移语句加入 end
            end_index.insert(code_idx);
            // 条件转移语句的下一条加入start
            if(q1_op != OP_j){
                start_index.insert(code_idx+1);
            }
        }
        // 存储四元式
        INPUT_CODE input_code;
        input_code.index = code_idx;
        input_code.code.q1_op = q1_op;
        input_code.code.q2_in1 = q2_in1;
        input_code.code.q3_in2 = q3_in2;
        input_code.code.q4_out = q4_out;
        input_code_buf.push(input_code);
        counter+=1;
    }
    fclose(fp);
    return true;
}

// 进行代码块的划分
bool code2block()
{
    vector<int> start_index_vector;
    vector<int> end_index_vector;
    for(auto iter = start_index.begin(); iter != start_index.end(); ++iter){
        start_index_vector.push_back(*iter);
    }
    for(auto iter = end_index.begin(); iter != end_index.end(); ++iter){
        end_index_vector.push_back(*iter);
    }
    // 进行一个排序
    sort(start_index_vector.begin(), start_index_vector.end());
    sort(end_index_vector.begin(), end_index_vector.end());
    // 两个指针
    int st_p = 0;
    int ed_p = 0;
    int st_len = start_index_vector.size();
    int ed_len = end_index_vector.size();
    //int last_block_idx = -1; // 作为前驱的上一个代码块
    while(1){
        int st_now = start_index_vector[st_p];
        int ed_now = -1;

        if(st_p+1 < st_len && ed_p < ed_len && start_index_vector[st_p + 1]-1 < end_index_vector[ed_p])
            ed_now = start_index_vector[st_p + 1] - 1;
        else if( ed_p < ed_len )
            ed_now = end_index_vector[ed_p++];
        else if( st_p+1 < st_len )
            ed_now = start_index_vector[st_p + 1] - 1;
        printf("st_now = %d\ted_now = %d\n", st_now, ed_now);
        // 至此，已确定 st_now 和 ed_now，其中 ed_now == -1 则表示当前块为最后一块
        // 接下来，根据 st_now 和 ed_now 从 input_code_buf 中取出代码，构造 CodeBlock
        CodeBlock cb;
        // 检查是否需要设置label
        for(auto iter = label_map.begin(); iter != label_map.end(); ++iter){
            if(iter->second > 0 && iter->second <= st_now ){
                cb.labels.push_back(iter->first);
                iter->second = - code_blocks.size(); //将label_map指向所属代码块的index
            }
        }
        // for(auto iter = cb.labels.begin(); iter != cb.labels.end(); ++iter){
        //     label_map.erase(*iter);
        // }

        // 从 input_code_buf 中取出代码
        INPUT_CODE icd;
        bool exit_flag = true;
        while(!input_code_buf.empty()){
            icd = input_code_buf.front();
            if(ed_now < 0 || icd.index <= ed_now){
                // 检查代码是否为转移语句
                if (icd.code.q1_op == OP_j   ||
                    icd.code.q1_op == OP_jnz ||
                    icd.code.q1_op == OP_jeq ||
                    icd.code.q1_op == OP_jne ||
                    icd.code.q1_op == OP_jl  ||
                    icd.code.q1_op == OP_jle ||
                    icd.code.q1_op == OP_jg  ||
                    icd.code.q1_op == OP_jge )
                {
                    int idx = -label_map["L"+icd.code.q4_out];
                    if(idx < 0){
                        idx = - atoi(icd.code.q4_out.c_str());
                    }
                    cb.nbs_idx.push_back(idx);
                }
                cb.codes.push_back(icd.code);
                input_code_buf.pop();
            }else{
                exit_flag = false;
                break; // 取代码结束
            }
        }
        // 将下一个代码块索引加入该代码块的nbs_idx
        if((ed_now >= 0) && (cb.codes.back().q1_op != OP_j)){
            cb.nbs_idx.push_back(code_blocks.size()+1);
        }
        // 更新cb
        if(ed_now < 0)
            ed_now = icd.index;
        cb.st_code_index = st_now;
        cb.ed_code_index = ed_now;
        code_blocks.push_back(cb);
        if(exit_flag)
            break;
        st_p+=1;
    }
    // 第二个循环，处理所有小于0的nbs_idx
    for(auto iter = code_blocks.begin(); iter < code_blocks.end(); ++iter){
        for(auto idx_iter = iter->nbs_idx.begin(); idx_iter < iter->nbs_idx.end(); ++idx_iter){
            if(*idx_iter < 0){
                stringstream ss;
                ss << "L" << (-*idx_iter);
                *idx_iter = -label_map[ss.str()];
            }
        }
    }
    return true;
}

// 检查ncb_idx是否存在 left_vs
bool chk_param(set<string> & left_vs, int cb_idx, int ncb_idx, set<int>&trace_idxes)
{
    // 先检查自己这里
    for(auto code_iter = code_blocks[ncb_idx].codes.begin(); code_iter < code_blocks[ncb_idx].codes.end(); ++code_iter){
        if(left_vs.find(code_iter->q2_in1) != left_vs.end()){
            code_blocks[cb_idx].out_active.insert(code_iter->q2_in1);
        }
        if(left_vs.find(code_iter->q3_in2) != left_vs.end()){
            code_blocks[cb_idx].out_active.insert(code_iter->q3_in2);
        }
    }
    // 再检查后继
    for(auto next_cb = code_blocks[ncb_idx].nbs_idx.begin(); next_cb < code_blocks[ncb_idx].nbs_idx.end(); ++next_cb){
        // 若为起始cb，则跳过
        if(*next_cb == cb_idx)
            continue;
        if(trace_idxes.find(*next_cb)!=trace_idxes.end())
            continue;
        trace_idxes.insert(*next_cb);
        chk_param(left_vs, cb_idx, *next_cb, trace_idxes);
    }
    return true;
}

// 确定每个块中的出口活跃变量
bool set_active_param()
{
    int cb_len = code_blocks.size();
    for(int i = 0; i < cb_len; ++i){
        // 1. 收集 cb 中的所有左值符号
        set<string> left_vs;
        for (auto code_iter = code_blocks[i].codes.begin(); code_iter < code_blocks[i].codes.end(); ++code_iter)
            if (code_iter->q4_out != "_" && code_iter->q4_out.substr(1,2) != "SP")
                left_vs.insert(code_iter->q4_out);
        // 2. 检查所有后继代码块（深度优先搜索）
        set<int> trace_idxes;
        trace_idxes.clear();
        for (auto next_cb = code_blocks[i].nbs_idx.begin(); next_cb < code_blocks[i].nbs_idx.end(); ++next_cb)
        {
            if(*next_cb == i)
                continue;
            if(trace_idxes.find(*next_cb) != trace_idxes.end())
                continue;
            trace_idxes.insert(*next_cb);
            chk_param(left_vs, i, *next_cb, trace_idxes);
        }
    }
    return true;
}

bool isNumber(const string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

// 为每个代码块生成待用活跃信息表
bool set_ua_table()
{
    for(auto cb_iter = code_blocks.begin(); cb_iter < code_blocks.end(); ++cb_iter){
        // 1. 初始化：找到所有变量，初始化 vua_table
        for(auto code_iter = cb_iter->codes.begin(); code_iter < cb_iter->codes.end(); ++code_iter){
            ua tmp_ua={-1, false};
            if(!isNumber(code_iter->q2_in1) && code_iter->q2_in1 != "_"){
                // 是出口活跃变量
                if(cb_iter->out_active.find(code_iter->q2_in1) != cb_iter->out_active.end()){
                    tmp_ua.active = true;
                }
                (cb_iter->vua_table).v_ua_tb[code_iter->q2_in1] = tmp_ua;
            }
            if (!isNumber(code_iter->q3_in2) && code_iter->q3_in2 != "_")
            {
                if (cb_iter->out_active.find(code_iter->q3_in2) != cb_iter->out_active.end()){
                    tmp_ua.active = true;
                }
                (cb_iter->vua_table).v_ua_tb[code_iter->q3_in2] = tmp_ua;
            }
            if (!isNumber(code_iter->q4_out) && code_iter->q4_out != "_")
            {
                if (cb_iter->out_active.find(code_iter->q4_out) != cb_iter->out_active.end()){
                    tmp_ua.active = true;
                }
                (cb_iter->vua_table).v_ua_tb[code_iter->q4_out] = tmp_ua;
            }
        }
        // 2. 从后往前处理各个四元式
        int codes_len = cb_iter->codes.size();
        //进行初始化qua_table
        cb_iter->qua_table.init(codes_len);
        for(int i = codes_len-1; i >= 0; --i){
            string q4_out = cb_iter->codes[i].q4_out;
            string q2_in1 = cb_iter->codes[i].q2_in1;
            string q3_in2 = cb_iter->codes[i].q3_in2;
            if(isNumber(q4_out) || q4_out =="_")
                q4_out = "";
            if (isNumber(q2_in1) || q2_in1 == "_")
                q2_in1 = "";
            if (isNumber(q3_in2) || q3_in2 == "_")
                q3_in2 = "";
            // 更新 qua_table
            if(q4_out!="")
                cb_iter->qua_table.out_ua_tb[i] = cb_iter->vua_table.v_ua_tb[q4_out];
            if(q2_in1!="")
                cb_iter->qua_table.in1_ua_tb[i] = cb_iter->vua_table.v_ua_tb[q2_in1];
            if(q3_in2!="")
                cb_iter->qua_table.in2_ua_tb[i] = cb_iter->vua_table.v_ua_tb[q3_in2];
            // 更新 vua_table
            if (q4_out != "")
                cb_iter->vua_table.v_ua_tb[q4_out] = {-1, false};
            if (q2_in1 != "")
                cb_iter->vua_table.v_ua_tb[q2_in1] = {i, true};
            if (q3_in2 != "")
                cb_iter->vua_table.v_ua_tb[q3_in2] = {i, true};
        }
    }
    return true;
}

// 判断该变量是否为内存变量
// 若是，则将寄存器中的值同步至内存
void update_vname_reg2mem(string vname, fstream& fout_mips_code)
{
    //寄存器，检查是否关联内存变量
    if(RVAULE.regs_map.find(vname) != RVAULE.regs_map.end()){
        vname = RVAULE.regs_map[vname].value;
    }
    
    
    if(vname.substr(0,1)=="[" && vname.substr(1,2)!="SP"){
        //是内存变量，将寄存器中的内容同步至内存
        string reg_name = "";
        string mem_name = "";
        for(auto addr = AVALUE[vname].begin(); addr < AVALUE[vname].end(); ++addr){
            if(addr->is_reg)
                reg_name = addr->addr;
            else
                mem_name = addr->addr;
        }
        if(reg_name!="" && mem_name!=""){
            fout_mips_code << "\tsw " << reg_name << ", " << mem_name << "\n";
        }
    }
}

// 申请一个寄存器
string alloc_reg(int cb_idx, fstream& fout_mips_code)
{
    string reg_name = "";
    // 1. 尝试找一个空闲寄存器
    reg_name = RVAULE.find_empty_reg();
    if (reg_name != "")
        return reg_name;
    // 2. 从已分配寄存器中选一个（变量同时在内存，变量在最远处被引用）
    // * 变量同时在内存的
    for (auto reg = RVAULE.regs_map.begin(); reg != RVAULE.regs_map.end(); ++reg){
        string vname = reg->second.value;
        for (auto vaddr = AVALUE[vname].begin(); vaddr < AVALUE[vname].end(); ++vaddr){
            // 在内存中存在
            if (!vaddr->is_reg){
                fout_mips_code << "\tsw " << reg->first << ", " << vaddr->addr << "\n";
                return reg->first;
            }
        }
    }
    //* 变量不在内存，但是在最远处被引用
    printf("[ERROR]alloc_reg失败，请完善实现\n");
    return "";
}

// 给一个变量分配寄存器
string get_vreg(int cb_idx, int code_idx, fstream& fout_mips_code, string vname)
{
    //* 如果是数字（分配一个寄存器，放入寄存器中，返回寄存器名称）
    if (isNumber(vname))
    {
        string reg_name = alloc_reg(cb_idx, fout_mips_code);
        fout_mips_code << "\tli " << reg_name << ", " << vname << "\n";
        return reg_name;
    }
    //* 如果不是数字
    //     1. 在寄存器中（返回寄存器名称）
    //     2. 在栈中（生成栈地址，分配一个寄存器，放入寄存器中，返回寄存器名称）
    //     3. 在内存中（分配一个寄存器，放入寄存器中，返回寄存器名称）
    // 在寄存器或内存中
    auto q_addrs = AVALUE[vname];
    string reg_name = "";
    string mem_name = "";
    for (auto addr_iter = q_addrs.begin(); addr_iter < q_addrs.end(); ++addr_iter)
    {
        if (addr_iter->is_reg)
            reg_name = addr_iter->addr;
        else
            mem_name = addr_iter->addr;
    }
    if (reg_name != "")
        return reg_name; // 在寄存器中
    if (mem_name != ""){ // 不在寄存器中，但是在内存中
        // 分配一个寄存器
        reg_name = alloc_reg(cb_idx, fout_mips_code);
        // 将变量lw到寄存器
        fout_mips_code << "\tlw " << reg_name << ", " << mem_name << "\n";
        return reg_name;
    }
    // 在栈中
    if (vname.substr(1, 2) == "SP")
    {
        //在栈中[SP+0][SP+1][SP+2]
        string reg_name = "";
        stringstream ss;
        ss.str("");
        int sp_idx;
        ss << vname.substr(4);
        ss >> sp_idx;
        ss.str("");
        ss << (-4 * (1 + sp_idx)) << "($fp)";
        reg_name = alloc_reg(cb_idx, fout_mips_code);
        fout_mips_code << "\tlw " << reg_name << ", " << ss.str() << "\n";
        return reg_name;
    }
    // 运行到这里属于是有问题
    printf("[ERROR]get_vreg出错，请调试.\n");
    return "";
}

// 更新AVALUE RVALUE信息 :
// 1. reg_map 映射至 vname
// 2. vname的addr加入reg_name
// 3. 丢掉vname的addr中的其他reg_name
// 4. reg_map 原来映射的取消关联
bool update_reginfo(string reg_name, string vname)
{
    if(!RVAULE.regs_map[reg_name].is_empty){
        string old_vname = RVAULE.regs_map[reg_name].value;
        if (old_vname!=vname){
            vector<AddrV> new_addr;
            for(auto addr = AVALUE[old_vname].begin(); addr < AVALUE[old_vname].end(); ++addr){
                if(addr->is_reg && addr->addr == reg_name)
                    continue;
                new_addr.push_back(*addr);
            }
            AVALUE[old_vname] = new_addr;
        }
    }

    RVAULE.regs_map[reg_name].value = vname;
    RVAULE.regs_map[reg_name].is_empty = false;

    if (AVALUE.find(vname) == AVALUE.end())
    {
        vector<AddrV> addrv;
        addrv.push_back({true, reg_name});
        AVALUE[vname] = addrv;
    }
    else
    {
        vector<AddrV> new_addr;
        for (auto addr = AVALUE[vname].begin(); addr < AVALUE[vname].end(); ++addr){
            if(!addr->is_reg){
                new_addr.push_back(*addr);
            }else if(addr->is_reg && addr->addr!=reg_name){
                if(RVAULE.regs_map[addr->addr].value == vname){
                    RVAULE.regs_map[addr->addr].value = "";
                    RVAULE.regs_map[addr->addr].is_empty = true;
                }
            }
        }
        new_addr.push_back({true, reg_name});
        AVALUE[vname] = new_addr;
    }
    return true;
}

// op_out 寄存器分配算法
string op_get_reg(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    string reg_name = "";
    auto code = code_blocks[cb_idx].codes[code_idx];
    string in1_addr = code.q2_in1;
    if (!isNumber(code.q2_in1))
    { // 如果不是数值
        // 获取in1的活跃待用信息
        auto in1_ua = code_blocks[cb_idx].qua_table.in1_ua_tb[code_idx];
        for (auto q2_addr = AVALUE[code.q2_in1].begin(); q2_addr < AVALUE[code.q2_in1].end(); ++q2_addr)
        {
            if (q2_addr->is_reg)
            {
                in1_addr = q2_addr->addr;
                if(!in1_ua.active || code.q4_out == code.q2_in1){
                    reg_name = q2_addr->addr;
                    update_reginfo(reg_name, code.q4_out);
                    return reg_name;
                }
                break; // in1活跃且在寄存器当中
            }else{
                in1_addr = q2_addr->addr; // in1在内存中
            }
        }
    }
    //** 至此，表示in1不在reg中，或者in1在此处是活跃的，或者in1是常数
    // 1. 尝试找一个空闲寄存器
    reg_name = RVAULE.find_empty_reg();
    if (reg_name != ""){
        if(isNumber(in1_addr))
            fout_mips_code << "\tli " << reg_name << ", " << in1_addr << "\n";
        else if(in1_addr.substr(0,1) == "$")
            fout_mips_code << "\tmove " << reg_name << ", " << in1_addr << "\n";
        else
            fout_mips_code << "\tlw " << reg_name << ", " << in1_addr << "\n";
        update_reginfo(reg_name, code.q4_out);
        return reg_name;
    }
    // 2. 从已分配寄存器中选一个（变量同时在内存，变量在最远处被引用）
    // 找一个变量同时在内存的
    for (auto reg = RVAULE.regs_map.begin(); reg != RVAULE.regs_map.end(); ++reg)
    {
        string vname = reg->second.value;

        for (auto vaddr = AVALUE[vname].begin(); vaddr < AVALUE[vname].end(); ++vaddr)
        {
            // 在内存中存在
            if (!vaddr->is_reg)
            {

                fout_mips_code << "\tsw " << reg->first <<", " << vaddr->addr << "\n";
                update_reginfo(reg_name, code.q4_out);
                return reg->first;
            }
        }
    }
    // 变量不在内存，则需要先将其存入内存（待实现）
    printf("[ERROR] op_get_reg 寄存器分配失败，请实现相关功能!\n");
    return "";
}

// 代码生成：运算 add sub mult div
bool gen_op_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];

    // 处理四元式，为 q4_out 分配寄存器
    string reg_name = op_get_reg(cb_idx, code_idx, fout_mips_code);
    if (reg_name == "")
        return false;

    // 取得 q2 当前存放的位置
    string in2_addr = code.q3_in2;
    if(!isNumber(in2_addr)){
        in2_addr = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
        if(in2_addr == "")
            return false;
        update_reginfo(in2_addr, code.q3_in2);
    }

    // 根据op生成相应的处理语句
    if(code.q1_op == OP_add){
        fout_mips_code << "\tadd "  << reg_name << ", " <<reg_name<< ", " << in2_addr <<"\n";
    }else if(code.q1_op == OP_sub){
        fout_mips_code << "\tsub "  << reg_name << ", " <<reg_name<< ", " << in2_addr <<"\n";
    }else if(code.q1_op == OP_mul){
        fout_mips_code << "\tmult " << reg_name << ", " <<in2_addr << "\n";
        // 把lo寄存器中的值放入 reg_name
        fout_mips_code << "\tmflo " << reg_name << "\n";
    }else if(code.q1_op == OP_dev){
        fout_mips_code << "\tdiv "  << reg_name << ", " << in2_addr << "\n";
        fout_mips_code << "\tmflo " << reg_name << "\n";
    }

    // 若q4_out为内存变量，则将其值同步至内存
    update_vname_reg2mem(code.q4_out, fout_mips_code);

    return true;
}

// 代码生成：赋值
bool gen_fuzhi_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];
    // 检查 out
    // 是内存变量：1.在reg中，2.不在reg中
    // 是临时变量：1.在reg中，2.不在reg中
    string out_reg_name = "";
    for(auto addr = AVALUE[code.q4_out].begin(); addr < AVALUE[code.q4_out].end(); ++addr){
        if(addr->is_reg)
            out_reg_name = addr->addr;
    }
    if(out_reg_name != ""){
        if(isNumber(code.q2_in1)){
            fout_mips_code << "\tli " << out_reg_name << ", " << code.q2_in1 << "\n";
        }else{
            string reg_name = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
            update_reginfo(reg_name, code.q2_in1);
            fout_mips_code << "\tmove " << out_reg_name << ", " << reg_name << "\n";
        }
        update_vname_reg2mem(code.q4_out, fout_mips_code);
        return true;
    }
    
    // 将in1放入寄存器中
    string reg_name = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
    // 修改这个寄存器的 RVALUE (置为out)
    update_reginfo(reg_name, code.q4_out);
    // 检查out是否为出口活跃变量
    update_vname_reg2mem(code.q4_out, fout_mips_code);
    return true;
}

// 代码生成：跳转
bool gen_j_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];
    if(code.q1_op == OP_j){
        fout_mips_code << "\tj " << "L" << code.q4_out << "\n";
    }
    else if(code.q1_op == OP_jne){
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if(q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if(!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if(q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tbne " << q2_in1 << ", " << q3_in2 << ", L"<< code.q4_out <<"\n";
    }
    else if(code.q1_op == OP_jeq){
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if (q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if (!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if (q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tbeq " << q2_in1 << ", " << q3_in2 << ", L"<< code.q4_out <<"\n";
    }
    else if(code.q1_op == OP_jnz){
        string q2_in1 = code.q2_in1;
        if(!isNumber(q2_in1)){
            q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
            if (q2_in1 == "")
                return false;
            update_reginfo(q2_in1, code.q2_in1);
        }
        fout_mips_code << "\tbne $zero, " << q2_in1 << ", L" << code.q4_out <<"\n";
    }
    else if(code.q1_op == OP_jg){
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if (q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if (!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if (q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tbgt " << q2_in1 << ", " << q3_in2 << ", L" << code.q4_out << "\n";
    }
    else if (code.q1_op == OP_jge)
    {
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if (q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if (!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if (q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tbge " << q2_in1 << ", " << q3_in2 << ", L" << code.q4_out << "\n";
    }
    else if (code.q1_op == OP_jl)
    {
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if (q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if (!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if (q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tblt " << q2_in1 << ", " << q3_in2 << ", L" << code.q4_out << "\n";
    }
    else if (code.q1_op == OP_jle)
    {
        string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if (q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        string q3_in2 = code.q3_in2;
        if (!isNumber(code.q3_in2)){
            q3_in2 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q3_in2);
            if (q3_in2 == "")
                return false;
            update_reginfo(q3_in2, code.q3_in2);
        }
        fout_mips_code << "\tble " << q2_in1 << ", " << q3_in2 << ", L" << code.q4_out << "\n";
    }
    return true;
}

// 代码生成：函数调用 param
bool gen_param_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];
    // subi $sp 4
    string q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
    if(q2_in1 == "")
        return false;
    fout_mips_code << "\tsubi $sp, $sp, 4\n";
    fout_mips_code << "\tsw " << q2_in1 << ", 0($sp)\n";
    return true;
}

void push_regs(fstream& fout_mips_code)
{
    for(int i = 0; reg_names[i]!=NULL; ++i){
        fout_mips_code << "\tsubi $sp, $sp, 4\n";
        fout_mips_code << "\tsw " << reg_names[i] << ", 0($sp)\n";
    }
}
void pop_regs(fstream& fout_mips_code)
{
    int reg_len = 0;
    while(reg_names[reg_len]!=NULL)
        reg_len+=1;

    for(int i = reg_len-1; i>=0; --i){
        fout_mips_code << "\tlw " << reg_names[i] << ", 0($sp)\n";
        fout_mips_code << "\taddi $sp, $sp, 4\n";
    }

}

// 代码生成：函数调用 call
bool gen_call_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];
    //函数调用前的处理
    fout_mips_code << "\tsubi $sp, $sp, 4\n";
    fout_mips_code << "\tsw $ra, 0($sp)\n";
    fout_mips_code << "\tsubi $sp, $sp, 4\n";
    fout_mips_code << "\tsw $fp, 0($sp)\n";
    //根据被调用函数参数的个数，调整$fp的指向
    stringstream ss;
    ss << (2+func_map[code.q2_in1])*4;
    fout_mips_code << "\taddi $fp, $sp, " << ss.str() << "\n";
    //保存所有寄存器的值到栈中
    push_regs(fout_mips_code);
    //函数调用
    fout_mips_code << "\tjal " << code.q2_in1 << "\n";
    //恢复所有寄存器的值
    pop_regs(fout_mips_code);
    //函数返回后的处理
    fout_mips_code << "\tlw $v1, 0($sp)\n";
    fout_mips_code << "\tlw $ra, 4($sp)\n";
    fout_mips_code << "\tmove $sp, $fp\n";
    fout_mips_code << "\tmove $fp, $v1\n";
    //函数有返回值，且被使用
    if(code.q4_out != "_"){
        // 检查q4_out是否在寄存器中
        string reg_name = "";
        string mem_name = "";
        for(auto a = AVALUE[code.q4_out].begin(); a < AVALUE[code.q4_out].end(); ++a){
            if(a->is_reg)
                reg_name = a->addr;
            else
                mem_name = a->addr;
        }
        //在寄存器中
        if(reg_name != ""){
            fout_mips_code << "\tmove " << reg_name << ", $v0\n";
        }
        //在内存但不在寄存器中 [1]
        if(mem_name != ""){
            fout_mips_code << "\tsw $v0, " << mem_name << "\n";
        }
        //不在寄存器也不在内存中 T1 : 分配一个寄存器给它
        reg_name = alloc_reg(cb_idx, fout_mips_code);
        if(reg_name == ""){
            return false;
        }
        update_reginfo(reg_name, code.q4_out);
        fout_mips_code << "\tmove " << reg_name << ", $v0\n";
    }
    return true;
}

// 代码生成：函数调用 ret
bool gen_ret_code(int cb_idx, int code_idx, fstream& fout_mips_code)
{
    FOUR_CODE code = code_blocks[cb_idx].codes[code_idx];
    string q2_in1 = code.q2_in1;
    if(!isNumber(q2_in1)){
        q2_in1 = get_vreg(cb_idx, code_idx, fout_mips_code, code.q2_in1);
        if(q2_in1 == "")
            return false;
        update_reginfo(q2_in1, code.q2_in1);
        fout_mips_code << "\tmove $v0, " << q2_in1 << "\n";
    }else{
        fout_mips_code << "\tli $v0, " << q2_in1 << "\n";
    }
    fout_mips_code << "\tjr $ra\n";

    return true;
}

// 块初始化：
// 将内存变量、栈变量信息写入AVALUE
bool init_cb_gen_code(int cb_idx)
{
    auto codes = code_blocks[cb_idx].codes;
    for(auto code = codes.begin(); code < codes.end(); ++code){
        vector<string> vnames;
        vnames.clear();
        vnames.push_back(code->q2_in1);
        vnames.push_back(code->q3_in2);
        vnames.push_back(code->q4_out);
        for(auto vname = vnames.begin(); vname < vnames.end(); ++vname){
            vector<AddrV> new_addrs;
            new_addrs.clear();
            // 栈变量（重新）
            if(vname->substr(1,2) == "SP"){
                stringstream ss;
                ss.str("");
                int sp_idx;
                ss << vname->substr(4);
                ss >> sp_idx;
                ss.str("");
                ss << (-4 * (1 + sp_idx)) << "($fp)";
                new_addrs.push_back({false, ss.str()});
                AVALUE[*vname] = new_addrs;
            }
            // 内存变量（延续）
            else if(vname->substr(0,1) == "["){
                // 如果不存在
                if(AVALUE.find(*vname) == AVALUE.end()){
                    stringstream ss;
                    ss.str("");
                    int mem_idx;
                    ss << vname->substr(1);
                    ss >> mem_idx;
                    ss.str("");
                    ss << (4*mem_idx) << "($a0)";
                    new_addrs.push_back({false, ss.str()});
                    AVALUE[*vname] = new_addrs;
                }
            }
        }
    }
    return true;
}

// 块结束：
// 清除除出口活跃变量之外的变量RVALUE AVALUE
// 将非出口活跃的内存变量写回内存
bool fini_cb_gen_code(int cb_idx, fstream& fout_mips_out)
{
    auto out_active = code_blocks[cb_idx].out_active;
    vector<string> dels;
    for(auto v = AVALUE.begin(); v != AVALUE.end(); ++v){
        // 不在出口活跃变量中
        if(out_active.find(v->first) == out_active.end()){
            dels.push_back(v->first);
            // // 属于非出口活跃的内存变量
            // if(v->first.substr(0,1) == "[" && v->first.substr(1,2)!="SP"){
            //     // 检查寄存器中是否有
            //     string reg_name = "";
            //     string mem_name = "";
            //     for(auto addr = v->second.begin(); addr < v->second.end(); ++addr){
            //         if(addr->is_reg){
            //             reg_name = addr->addr;
            //         }else{
            //             mem_name = addr->addr;
            //         }
            //     }
            //     if(reg_name!="" && mem_name!=""){
            //         fout_mips_out << "\tsw " << reg_name << ", " << mem_name <<"\n";
            //     }
            // }
        }
    }
    for(auto del = dels.begin(); del < dels.end(); ++del)
        AVALUE.erase(*del);

    for(auto r = RVAULE.regs_map.begin(); r != RVAULE.regs_map.end(); ++r){
        if(r->second.is_empty)
            continue;
        // 不在出口活跃变量中
        if(out_active.find(r->second.value) == out_active.end()){
            r->second.value = "";
            r->second.is_empty = true;
        }
    }
    return true;
}

void end_gen_code(fstream& fout_mips_code)
{
    // 将AVALUE中在寄存器中的内存变量存入内存
    for (auto v = AVALUE.begin(); v != AVALUE.end(); ++v)
    {
        if (v->first.substr(0, 1) == "[" && v->first.substr(1, 2) != "SP")
        {
            if (v->second.size() < 2)
                continue;
            string reg_name = "";
            string mem_name = "";
            for (auto addr = v->second.begin(); addr < v->second.end(); ++addr)
            {
                if (addr->is_reg)
                {
                    reg_name = addr->addr;
                }
                else
                {
                    mem_name = addr->addr;
                }
            }
            if (reg_name != "" && mem_name != "")
            {
                fout_mips_code << "sw " << reg_name << ", " << mem_name << "\n";
            }
        }
    }
    //程序结束
    fout_mips_code << "li $v0, 10\nsyscall\n";
}

// 根据代码块生成汇编代码
bool generate_mips_code(string mips_fpath)
{
    RVAULE.init();
    // 1. 数据定义部分
    fstream fout_mips_code;
    fout_mips_code.open(mips_fpath, std::ios::out);
    fout_mips_code  << ".data\n\tmem: .space " << mem_param_num << "\n";
    fout_mips_code  << ".text\n"
                    << ".globl main\n"
                    << "la $a0, mem\n"
                    << "li $sp, 268697600\n"
                    << "li $fp, 268697600\n"
                    << "jal main\n";

    int cb_idx = 0;
    // 2. 根据代码块生成代码
    for(auto cb_iter=code_blocks.begin(); cb_iter < code_blocks.end(); ++cb_iter, ++cb_idx)
    {
        // ** 1. 生成代码块的标签
        int  F_param_num = 0;
        for(auto label_iter = cb_iter->labels.begin(); label_iter < cb_iter->labels.end(); ++label_iter){
            fout_mips_code << *label_iter << ":\n";
            if(func_map.find(*label_iter) != func_map.end()){
                F_param_num = func_map[*label_iter];
            }
        }
        // ** 2. AVALUE 初始化 : 将内存变量、栈变量信息写入AVALUE
        init_cb_gen_code(cb_idx);
        // ** 3. 逐行处理代码块中的代码
        int code_idx = 0;
        for(auto code_iter= cb_iter->codes.begin(); code_iter < cb_iter->codes.end(); ++code_iter, ++code_idx){
            bool ret_status;
            if(code_iter->q1_op == OP_add || code_iter->q1_op == OP_sub || code_iter->q1_op == OP_mul || code_iter->q1_op == OP_dev)
                ret_status = gen_op_code(cb_idx, code_idx, fout_mips_code);
            else if(code_iter->q1_op == OP_j || code_iter->q1_op == OP_jeq || code_iter->q1_op == OP_jne || code_iter->q1_op == OP_jnz ||
                    code_iter->q1_op == OP_jg || code_iter->q1_op == OP_jge || code_iter->q1_op == OP_jl || code_iter->q1_op == OP_jle)
                ret_status = gen_j_code(cb_idx, code_idx, fout_mips_code);
            else if(code_iter->q1_op == OP_fuzhi)
                ret_status = gen_fuzhi_code(cb_idx, code_idx, fout_mips_code);
            else if(code_iter->q1_op == OP_param)
                ret_status = gen_param_code(cb_idx, code_idx, fout_mips_code);
            else if(code_iter->q1_op == OP_call)
                ret_status = gen_call_code(cb_idx, code_idx, fout_mips_code);
            else if(code_iter->q1_op == OP_ret)
                ret_status = gen_ret_code(cb_idx, code_idx, fout_mips_code);
            if (ret_status){
                printf("[INFO] 块%d句%d(%s,%s,%s,%s)\t生成成功\n", cb_idx, code_idx, code_iter->q1_op.c_str(), code_iter->q2_in1.c_str(), code_iter->q3_in2.c_str(), code_iter->q4_out.c_str());
            }
            else{
                printf("[INFO] 块%d句%d(%s,%s,%s,%s)\t生成失败!\n", cb_idx, code_idx, code_iter->q1_op.c_str(), code_iter->q2_in1.c_str(), code_iter->q3_in2.c_str(), code_iter->q4_out.c_str());
                return false;
            }
        }
        // 清除无关变量RVALUE AVALUE
        // 将出口活跃变量存入内存
        fini_cb_gen_code(cb_idx, fout_mips_code);
    }
    // 3. 结束代码
    end_gen_code(fout_mips_code);
    return true;
}


bool run_as()
{
    INFO as_info;
    if(!inputcode("./ilcode.txt")){
        as_info.tolog(_LOG_T_ERROR, "AS:读取并处理中间代码失败");
        return false;
    }
    if(!code2block()){
        as_info.tolog(_LOG_T_ERROR, "AS:划分代码块失败");
        return false;
    }else{
        as_info.tolog(_LOG_T_INFO, "AS:划分代码块成功");
    }
    if(!set_active_param()){
        as_info.tolog(_LOG_T_ERROR, "AS:出口活跃变量设置失败");
        return false;
    }else{
        as_info.tolog(_LOG_T_ERROR, "AS:出口活跃变量设置成功");
    }
    if(!set_ua_table()){
        as_info.tolog(_LOG_T_ERROR, "AS:活跃待用表生成失败");
        return false;
    }else{
        as_info.tolog(_LOG_T_INFO, "AS:活跃待用表生成成功");
    }

    // 打印所有代码块
    for(auto iter = code_blocks.begin(); iter < code_blocks.end(); ++iter){
        printf("\n=====================================\n");
        for(auto label_iter = iter->labels.begin(); label_iter < iter->labels.end(); ++label_iter){
            printf("%s ", (*label_iter).c_str());
        }
        printf("\n");
        for(auto code_iter = iter->codes.begin(); code_iter < iter->codes.end(); ++code_iter){
            printf("%s %s %s %s\n", (code_iter->q1_op).c_str(), (code_iter->q2_in1).c_str(), (code_iter->q3_in2).c_str(), (code_iter->q4_out).c_str());
        }
        printf("st_code_index:%d\ted_code_index:%d\n", iter->st_code_index, iter->ed_code_index);
        printf("next blocks index:\n");
        for(auto nb_iter = iter->nbs_idx.begin(); nb_iter < iter->nbs_idx.end(); ++nb_iter){
            printf("%d\t", *nb_iter);
        }
        printf("\nout_active_v:\n");
        for(auto out_active = iter->out_active.begin(); out_active != iter->out_active.end(); ++out_active){
            printf("%s\t", (*out_active).c_str());
        }
        printf("\n");
        iter->qua_table.print();
        iter->vua_table.print();
        fflush(stdout);
    }
    // 生成 mips 汇编代码
    if(!generate_mips_code("./mips.asm")){
        as_info.tolog(_LOG_T_ERROR, "AS:汇编代码生成失败");
        return false;
    }else{
        as_info.tolog(_LOG_T_INFO, "AS:汇编代码生成成功!");
    }
    return true;
}

/*
int main()
{
    if(inputcode("E:\\fqh_Workspace_back\\compiler_workspace\\lr_submit\\src\\LR\\ilcode.txt")){
        printf("start_index:");
        for (auto iter = start_index.begin(); iter != start_index.end(); ++iter)
            printf("%d ", *iter);
        printf("\n");
        printf("end_index:");
        for (auto iter = end_index.begin(); iter != end_index.end(); ++iter)
            printf("%d ", *iter);
        printf("\n");
        printf("label:\n");
        for (auto iter = label_map.begin(); iter != label_map.end(); ++iter)
            printf("%s\t%d\n", iter->first.c_str(), iter->second);

        // 开始划分代码块
        if(code2block() && set_active_param()){
            if(!set_ua_table()){
                printf("[ERROR] set_ua_table error!\n");
                return -1;
            }
            // 打印所有代码块
            for(auto iter = code_blocks.begin(); iter < code_blocks.end(); ++iter){
                printf("\n=====================================\n");
                for(auto label_iter = iter->labels.begin(); label_iter < iter->labels.end(); ++label_iter){
                    printf("%s ", (*label_iter).c_str());
                }
                printf("\n");
                for(auto code_iter = iter->codes.begin(); code_iter < iter->codes.end(); ++code_iter){
                    printf("%s %s %s %s\n", (code_iter->q1_op).c_str(), (code_iter->q2_in1).c_str(), (code_iter->q3_in2).c_str(), (code_iter->q4_out).c_str());
                }
                printf("st_code_index:%d\ted_code_index:%d\n", iter->st_code_index, iter->ed_code_index);
                printf("next blocks index:\n");
                for(auto nb_iter = iter->nbs_idx.begin(); nb_iter < iter->nbs_idx.end(); ++nb_iter){
                    printf("%d\t", *nb_iter);
                }
                printf("\nout_active_v:\n");
                for(auto out_active = iter->out_active.begin(); out_active != iter->out_active.end(); ++out_active){
                    printf("%s\t", (*out_active).c_str());
                }
                printf("\n");
                iter->qua_table.print();
                iter->vua_table.print();
            }

            // 开始生成代码
            if(generate_mips_code("./mips.asm")){
                printf("[INFO]生成mips汇编代码成功!\n");
            }else{
                printf("[ERROR]生成mips汇编代码失败!\n");
            }
        }
    }
    else{
        printf("读取并预处理中间代码失败\n");
    }
    return 0;
}
*/

vector<CodeBlock> get_codeblocks()
{
    return code_blocks;
}

