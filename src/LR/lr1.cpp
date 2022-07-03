
#include <stdio.h>
#include <iostream>
#include "../INFO/info.hpp"
#include "../CLEX/clex.hpp"
#include "./lr1.h"
using namespace std;

#define GPLIST_PATH "../test/c--add.gplst"
#define SOURCE_PATH "../test/test_func_array.txt"
#define ILCODE_PAHT "./ilcode.txt"

#define OP_j        "j"  
#define OP_jnz      "jnz"
#define OP_fuzhi    "="  
#define OP_jeq      "jeq"
#define OP_jne      "jne"
#define OP_jl       "j<" 
#define OP_jle      "j<="
#define OP_jg       "j>" 
#define OP_jge      "j>="
#define OP_add      "+"  
#define OP_sub      "-"  
#define OP_mul      "*"  
#define OP_dev      "/"  
#define OP_param    "param" 
#define OP_call     "call"  
#define OP_ret      "ret"   

/* 数据流示意 * 
- 符号表--> 文法产生式  
        --> 项集族  
        --> 规范 LR1 分析表  
-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
//词法分析器
static CLEX* myclex;
//::非终结符表
static NToken NT_list[] = {
    {"S0", "增广开始符"},
    {"S", "开始符"},
    {"T", "类型"},
    {"B", "语句块"},
    {"B1", "内部声明"},
    {"B2", "语句串"},
    {"G", "语句"},
    {"G=", "赋值语句"},
    {"Greturn", "return语句"},
    {"Gwhile", "while语句"},
    {"Gif", "if语句"},
    {"E", "表达式"},
    {"A", "加法表达式"},
    {"A+", "加法表达式后继"},
    {"relop", "关系运算符"},
    {"M", "项"},
    {"M+", "项后继"},
    {"N", "因子"},
    {"N+", "因子后继"},
    {"V", "变量声明"},
    {"KM", "空产生式使用"},
    {"KN", "空产生式使用"},
    {"Program", "整个单文件程序"},
    {"Fcs", "函数定义序列"},
    {"F", "单个函数定义"},
    {"Args", "参数定义序列"},
    {"Ids", "标识符序列"},
    {"AM", "空产生式使用"},
    {"BM", "空产生式使用"},
    {"Array", "数组下标"},
    {"", ""}};
//::终结符表 
//由词法分析器给出 [clex class]
//::符号表
static TokenManager all_token;
//::文法产生式表 
static GPlistManager GP_list_manager;
//::所有非终结符的FIRST集
static std::map<int, std::set<int>> all_nt_first_set;
//::项集族
static std::map<ITERMS, ITERMS_STATE> ITERMS_list;
//::规范LR语法分析表
static std::map<int, TableIterm> Action_Goto_Table;
//::状态栈
static std::stack<int> live_state;  //活前缀对应的状态
//::顶点栈(用于图像的绘制) 
static std::stack<int> live_dot;    //活顶点
static int dot_count;               //dot自增计数器
//::符号栈(用于语义检查与中间代码生成) 
static std::stack<Symbol> live_sym;
//::内存符号表(存储变量的相对地址) 
static MemSymTable sym_table_manager;
//::中间代码生成器 
static IL IL_code_manager;
//::函数体定义时，使用的形参表及形参栈
static std::map<std::string, FormParamTableItem> form_param_table; //形参表，函数定义开始时生成，函数体翻译结束后清空
static std::stack<FormParam> form_param_stack; //形参栈，翻译过程中使用，用于生成形参表
//::函数调用时，实参传入的实参栈
static std::stack<FormParam> real_param_stack;

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
/* CLOSURE(I)
    - 对I中所有false项目作闭包，并置ture，新项以false状态加入
    - 返回值为false时，函数将会 递 归 地 调用自己 (×)
    - 改为 open-close 法 , open==close时，结束！
-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static bool iterms_closure(ITERMS & in)
{
    unsigned long long open = 0;
    unsigned long long close = 0;
    //计算open
    for(auto iter = in.iterms.begin(); iter != in.iterms.end(); ++iter)
        if(!iter->second) open+=1;

    //开始closure
    do{ //退出条件：close == open
        for(std::map<LR1_ITERM, bool>::iterator iter = in.iterms.begin(); iter != in.iterms.end(); iter++){
            if(iter->second)
                continue;
            iter->second = true;
            close+=1;

            //检查当前项是否需要被扩展
            int RHS_size = GP_list_manager.GP_list[iter->first.GPi].RHS.size();
            if(RHS_size <= iter->first.dot)
                continue; //A->B.
            
            int dot_move = 0; // 因为 NT_epsilon 而移动的步数
            while(1){
                if(RHS_size <= iter->first.dot + dot_move)
                    break; //停止可扩展性检查 A->BBB.
                int vindex = GP_list_manager.GP_list[iter->first.GPi].RHS[iter->first.dot + dot_move];
                if( vindex/10000 ){ //* [A-> a.Bb,c] 点后面是非终结符，需要扩展
                    //取得B的下一个符号b或c
                    int next_vindex = -1; //1、-1 ; 2、非终结符的vindex; 3、终结符的vindex
                    int next_move_dot = 0;
                    while(RHS_size > iter->first.dot + 1 + next_move_dot){
                        int tmp = GP_list_manager.GP_list[iter->first.GPi].RHS[iter->first.dot + 1 + next_move_dot];
                        if(!GP_list_manager.NT_epsilon_vidset.count(tmp)){
                            next_vindex = tmp;
                            break;
                        }
                        next_move_dot+=1;
                    }
                    //对于每个产生式 B->r
                    int now_gpi = 0;
                    for(std::vector<GP>::iterator gp_iter = GP_list_manager.GP_list.begin(); gp_iter != GP_list_manager.GP_list.end(); ++gp_iter,++now_gpi){
                        if(gp_iter->LHS == vindex){
                            //1、构造被扩展的项
                            LR1_ITERM new_iterm;
                            new_iterm.GPi = now_gpi;
                            new_iterm.dot = 0;
                            //求 fw1_word_set
                            if(next_vindex == -1){         //将iter->fw1_word加入（传播）
                                new_iterm.fw1_word = iter->first.fw1_word;
                            }else if(next_vindex / 10000){ //将FIRST(next_vindex)加入（自发生成）
                                new_iterm.fw1_word = all_nt_first_set[next_vindex]; // 查NT_FIRST表 //
                            }else{ //将终结符 next_vindex 加入
                                new_iterm.fw1_word.insert(next_vindex);
                            }
                            //2、加入被扩展的项到项集中去
                            auto ret_insert = in.iterms.insert(make_pair(new_iterm, false));
                            if(ret_insert.second) 
                                open+=1; //新元素被添加
                        }
                    }
                }
                // 检查 vindex 是否为 NT_epsilon ，是则 dot_move+=1，继续检查
                // if( !GP_list_manager.NT_epsilon_vidset.count(vindex))
                    break;
                dot_move += 1;
            }
        }
    }while(open > close);
    return true;
}
//GOTO(I, X)
static bool iterms_goto(const ITERMS & I, int vindex, ITERMS & J)
{
    bool ret = false;
    for(std::map<LR1_ITERM, bool>::const_iterator iter = I.iterms.begin();iter!=I.iterms.end(); ++iter){
        int RHS_size = GP_list_manager.GP_list[iter->first.GPi].RHS.size();
        int dot_move = 0;
        while(1){
            if(RHS_size <= iter->first.dot + dot_move)
                break;
            //检查 next_vindex 是否为可转移的项
            int next_vindex = GP_list_manager.GP_list[iter->first.GPi].RHS[iter->first.dot + dot_move];
            if(next_vindex == vindex){
                ret = true;
                LR1_ITERM new_iterm;
                new_iterm.GPi = iter->first.GPi;
                new_iterm.dot = iter->first.dot + +dot_move + 1;
                new_iterm.fw1_word = iter->first.fw1_word;
                J.iterms.insert(make_pair(new_iterm, false));
            }
            //if(!GP_list_manager.NT_epsilon_vidset.count( next_vindex ))
                break;
            dot_move += 1;
        }
    }
    if(ret){ //对非空的 J 求闭包
        iterms_closure(J);
    }
    return ret;
}

//ITERMS: 求LR1项集族 ( 同时建立了规范LR1语法分析表 )
static bool get_iterms_list()
{   //ITERMS_list;
    //ITERMS_list.clear();
    //Action_Goto_Table.clear();
    map<ITERMS, ITERMS_STATE>().swap(ITERMS_list);
    map<int, TableIterm>().swap(Action_Goto_Table);
    //初始化
    INFO lr_info;
    ITERMS I0;
    LR1_ITERM start;
    start.GPi = 0;
    start.dot = 0;
    start.fw1_word.insert(all_token.get_wr_vindex("[结束符#]"));
    I0.iterms.insert(make_pair(start, false));
    iterms_closure(I0);
    //初始化索引自增器
    int I_index = 0;
    //I0 加入项集族
    ITERMS_list.insert(make_pair(I0, ITERMS_STATE{false, I_index++}));

    //开始构造
    unsigned int open  = 1; //I0
    unsigned int close = 0;
    do{
        for(std::map<ITERMS, ITERMS_STATE>::iterator iter=ITERMS_list.begin(); iter!=ITERMS_list.end(); ++iter){
            if(iter->second.isclose)
                continue;
            iter->second.isclose = true;
            close += 1;

            TableIterm table_iterm; 
            //对于每个文法符号
            for(int i = 0; all_token.NT_list[i].name!=""; ++i){
                int vid = all_token.get_NT_vindex(all_token.NT_list[i].name);
                ITERMS J;
                if( iterms_goto(iter->first, vid, J)){
                    int  J_index = I_index;
                    auto insert_ret = ITERMS_list.insert(make_pair(J, ITERMS_STATE{false, I_index++}));
                    if(insert_ret.second){  //插入成功
                        open += 1;
                    }else{                  //插入失败
                        I_index = J_index;  //* 恢复索引自增器
                        J_index = ITERMS_list[J].index; //* 取得已存在的 J 索引
                    }
                    //table_iterm.action_map 加入 (nt_vid, s_J)
                    auto goto_insert_ret = 
                    table_iterm.goto_map.insert(make_pair(vid, J_index)); 
                    //检查是否发生 移入 冲突 
                    if(!goto_insert_ret.second){
                        //tolog
                        ostringstream out;
                        out << "lr1 : 发生移入冲突 (I="      << iter->second.index 
                                                << " w="    << goto_insert_ret.first->first
                                                << " sa="   << goto_insert_ret.first->second
                                                << " sb="   << J_index << ").";
                        lr_info.tolog(_LOG_T_ERROR, out.str());
                        printf("发生移入冲突 (I=%d w=%d sa=%d sb=%d)",  iter->second.index, 
                                                                            goto_insert_ret.first->first,
                                                                            goto_insert_ret.first->second,
                                                                            J_index);
                    }
                    //printf("test: open = %d close = %d  insert_ret = %d vid = %d\n", open, close, insert_ret.second, vid);

                }
            }
            for(int i = 0; all_token.wr_list[i].type!=""; ++i){
                int vid = all_token.get_wr_vindex(all_token.wr_list[i].type);
                ITERMS J;
                if( iterms_goto(iter->first, vid, J)){
                    int  J_index = I_index;
                    auto insert_ret = ITERMS_list.insert(make_pair(J, ITERMS_STATE{false, I_index++}));
                    if(insert_ret.second){
                        open += 1;
                    }else{
                        I_index = J_index;
                        J_index = ITERMS_list[J].index;
                    }
                    //table_iterm.action_map 加入 (nt_vid, s_J)
                    auto action_insert_ret =  
                    table_iterm.action_map.insert(make_pair(vid, Action{ ACTION_T_S, J_index})); 
                    //检查是否发生 移入 冲突 
                    if(!action_insert_ret.second){
                        //tolog
                        ostringstream out;
                        out << "lr1 : 发生移入冲突 (I="      << iter->second.index 
                                                << " w="    << action_insert_ret.first->first
                                                << " sa="   << action_insert_ret.first->second.nx_id
                                                << " sb="   << J_index <<").";
                        lr_info.tolog(_LOG_T_ERROR, out.str());
                        printf("发生移入冲突 (I=%d w=%d sa=%d sb=%d)",  iter->second.index, 
                                                                            action_insert_ret.first->first,
                                                                            action_insert_ret.first->second.nx_id,
                                                                            J_index);
                    }
                    //printf("test: open = %d close = %d  insert_ret = %d vid = %d\n", open, close, insert_ret.second, vid);
                }
            }
            //检查当前状态的可归约性，将r*加入 Action语法分析表
            for(auto lr1_iter = iter->first.iterms.begin(); lr1_iter != iter->first.iterms.end(); ++lr1_iter){
                if(lr1_iter->first.is_r(&GP_list_manager)){
                    for(auto fw_iter = lr1_iter->first.fw1_word.begin(); fw_iter != lr1_iter->first.fw1_word.end(); ++fw_iter){
                        
                        auto action_insert_ret = 
                        table_iterm.action_map.insert(make_pair(*fw_iter, Action{ACTION_T_R, lr1_iter->first.GPi}));
                        
                        //检查是否发生冲突
                        if(!action_insert_ret.second){
                            //tolog
                            ostringstream out;
                            out << "lr1 : 发生" << ((action_insert_ret.first->second.action_type == ACTION_T_R)?"归约":"移入") << "-归约冲突"
                                << "(I=" << iter->second.index << " w=" << *fw_iter << " ga=" << action_insert_ret.first->second.nx_id << " gb=" << lr1_iter->first.GPi << ").";
                            lr_info.tolog(_LOG_T_ERROR, out.str());
                            printf("发生%s-归约冲突(I=%d w=%d ga=%d gb=%d)\n", (action_insert_ret.first->second.action_type == ACTION_T_R)? "归约":"移入",
                                                                                iter->second.index, *fw_iter, action_insert_ret.first->second.nx_id, lr1_iter->first.GPi);
                        }

                    }
                }
            }
            //将table_iterm加入Action_Goto_table
            Action_Goto_Table.insert(make_pair(iter->second.index, table_iterm));

        }
    }while( open > close);
    return true;
}
static void debug_print_iterms_list()
{
    printf("项集族共有 %d 项\n", ITERMS_list.size());
    int count = 0;
    for(auto ilst_iter = ITERMS_list.begin(); ilst_iter != ITERMS_list.end(); ++ilst_iter, ++count){
        auto iterms = ilst_iter->first.iterms;
        printf("[ %d ] I=%d\n", count, ilst_iter->second.index);
        for(auto iterm_iter = iterms.begin(); iterm_iter != iterms.end(); ++iterm_iter){
            
            int lhs_vid = GP_list_manager.GP_list[iterm_iter->first.GPi].LHS;
            string tname = all_token.NT_list[lhs_vid%10000].name;
            printf("%s -> ", tname.c_str());

            auto rhs_vid = GP_list_manager.GP_list[iterm_iter->first.GPi].RHS;
            int i;
            for(i = 0; i < (int)(rhs_vid.size()); ++i){
                if(i == iterm_iter->first.dot){
                    printf(".");
                }
                string wname;
                if(rhs_vid[i]/10000){
                    wname = all_token.NT_list[rhs_vid[i]%10000].name;
                }else{
                    wname = all_token.wr_list[rhs_vid[i]].type;
                }
                printf("%s ", wname.c_str());
            }
            if(i == iterm_iter->first.dot)
                printf(".");
            printf("\t\t\t{");
            auto fw1_set = iterm_iter->first.fw1_word;
            for(auto fw1_iter =  fw1_set.begin(); fw1_iter != fw1_set.end(); ++fw1_iter){
                printf(" %s ", all_token.wr_list[*fw1_iter].type.c_str());
            }
            printf("}\n");
        }
        printf("\n");
    }
}

//初始化: 非终结符的 FIRST 集合表
static void ini_all_nt_first_set(bool debug = 0)
{
    all_nt_first_set.clear();
    for(int i = 0; all_token.NT_list[i].name!=""; ++i){
        int vid = all_token.get_NT_vindex(all_token.NT_list[i].name);
        std::set<int> first_set = GP_list_manager.get_NT_FIRST(vid);
        all_nt_first_set.insert(make_pair(vid, first_set));
        if(debug){
            printf("%s\t\t%d : ", all_token.NT_list[i].name.c_str(), vid);
            printf("[");
            for(auto iter = first_set.begin(); iter != first_set.end(); ++iter)
                printf("%d ", *iter);
            printf("]\n");
        }
    }
}

//打印规范LR语法分析表
static void debug_print_Action_Goto_Table()
{
    for(auto titm_iter = Action_Goto_Table.begin(); titm_iter != Action_Goto_Table.end(); ++titm_iter){
        printf("I[%d] | ", titm_iter->first);
        for(auto action_iter = titm_iter->second.action_map.begin(); action_iter != titm_iter->second.action_map.end(); ++action_iter){
           printf("(%d, %s%d) ", action_iter->first, (action_iter->second.action_type == ACTION_T_S)?"s":"r", action_iter->second.nx_id);
        }
        for(auto goto_iter = titm_iter->second.goto_map.begin(); goto_iter != titm_iter->second.goto_map.end(); ++goto_iter){
            printf("[%d, %d] ", goto_iter->first, goto_iter->second);
        }
        printf("\n");
    }
}

//归约时画图
static bool graph_to_file(int GPi /*归约时使用的产生式序号*/)
{
    const char * tree_path = "./lr1_tree.dot";
    INFO lr_info;
    fstream fout;
    fout.open(tree_path, std::ios::app| std::ios::out);
    if(!fout.is_open()){
        ostringstream out;
        out << "lr1 : " << "绘制语法树时，打开文件失败 (path = " << tree_path << ").";
        lr_info.tolog(_LOG_T_ERROR, out.str());
        return false;
    }

    //节点的声明（除RHS的非终结符）
    stack<int> R_dot_index;
    //ostringstream edge_out;
    fout << dot_count << "[label=\"" << all_token.NT_list[GP_list_manager.GP_list[GPi].LHS % 10000].name << "\"]\n";
    //edge_out << dot_count << " -- " << "{";
    for(auto iter = GP_list_manager.GP_list[GPi].RHS.end(); iter != GP_list_manager.GP_list[GPi].RHS.begin(); --iter){
        if(!(*(iter-1) / 10000)){ //非终结符
            fout << live_dot.top() << "[label=\"" << all_token.wr_list[*(iter-1)].type << "\",shape=\"box\",color=\"#228b22\"]\n";
        }
        R_dot_index.push(live_dot.top());
        live_dot.pop();
    }
    fout << dot_count << " -- {";
    int Rsize = R_dot_index.size();
    for(int i = 0; i < Rsize; ++i){
        if(!i){
            fout << R_dot_index.top();
        }else{
            fout << "," <<R_dot_index.top();
        }
        R_dot_index.pop();
    }
    fout << "}\n";
    live_dot.push(dot_count++);
    
    fout.close();
    return true;
}
static bool graph_to_file_ini()
{
    const char * tree_path = "./lr1_tree.dot";
    INFO lr_info;
    fstream fout;
    fout.open(tree_path, std::ios::out);
    if(!fout.is_open()){
        ostringstream out;
        out << "lr1 : " << "绘制语法树时，打开文件失败 (path = " << tree_path << ").";
        lr_info.tolog(_LOG_T_ERROR, out.str());
        return false;
    }
    fout << "graph tree {\n";
    fout << "node [fontname=\"FangSong\"]\n";
    fout.close();
    return true;
}
static bool graph_to_file_end()
{
    const char * tree_path = "./lr1_tree.dot";
    INFO lr_info;
    fstream fout;
    fout.open(tree_path, std::ios::app|std::ios::out);
    if(!fout.is_open()){
        ostringstream out;
        out << "lr1 : " << "绘制语法树时，打开文件失败 (path = " << tree_path << ").";
        lr_info.tolog(_LOG_T_ERROR, out.str());
        return false;
    }
    fout << "}\n";
    fout.close();
    return true;
}

//打印内存符号表
static void debug_print_MemSymTable()
{
    ;
}

// 语义分析类型检查函数
/* 1、检查是否为说明语句归约 ---------------------
    V 	     [关键字int]	 [标识符]	[界符]				
    V 	     [关键字float]   [标识符]	[界符]
--------------------------------------------------*/
static bool sematype_chk_shuoming(int GPi)
{
    int LHS_idx = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    if(all_token.NT_list[LHS_idx%10000].name != "V")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if (RHS_size != 3)
        return false;
    live_sym.pop(); //界符
    Symbol sym_id = live_sym.top();
    live_sym.pop();
    Symbol sym_type = live_sym.top();
    live_sym.pop();

    //**说明语句语义分析：不产生四元组，更新 [ 内存符号表 ] 
    auto tb_find_rlt = sym_table_manager.tbs[sym_table_manager.cur_tbidx].find(sym_id.get_name());
    if (tb_find_rlt != sym_table_manager.tbs[sym_table_manager.cur_tbidx].end())
    {
        //出现变量名重定义：本语言，同一个过程调用中，不允许出现相同名称的变量
        printf("[Error] 出现变量重复定义! 可能出现未知错误...\n");
        return true;
    }
    //加入内存符号表
    sym_table_manager.add_item(sym_id.get_name(), sym_type.get_type());
    //左项加入符号栈
    Symbol V_sym;
    V_sym.add_name("V");
    V_sym.add_type(sym_type.get_type());
    live_sym.push(V_sym);

    return true;
}
/* 2、检查是否为说明语句（数组）归约 -----------------
    V        [关键字int]     [标识符]  Array  [界符]
    V        [关键字float]   [标识符]  Array  [界符]
--------------------------------------------------*/
void get_all_element(std::vector<int> & type_int_vec, int now_pos, string ss, vtype array_vtype)
{
    int n = type_int_vec.size();
    stringstream temp_ss;
    for(int i = 0; i < type_int_vec[now_pos]; ++i){
        // 当前这个维度
        temp_ss.str("");
        temp_ss << ss;
        temp_ss << "[" << i << "]";
        // n - back_i 之后的维度(递归)
        if(n == now_pos+1){ // 最后一个维度
            // 插入内存符号表
            if(sym_table_manager.find_item(temp_ss.str())){
                printf("[ERROR]发生语义错误，变量名重定义(%s)\n", temp_ss.str().c_str());
            }else{
                sym_table_manager.add_item(temp_ss.str(), array_vtype);
            }
        }else{
            // 继续补充维度
            get_all_element(type_int_vec, now_pos+1, temp_ss.str(), array_vtype);
        }
    }
    return;
}
static bool sematype_chk_shuoming_array(int GPi)
{
    int LHS_idx = GP_list_manager.GP_list[GPi].LHS; //产生式左项
    if (all_token.NT_list[LHS_idx % 10000].name != "V")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(RHS_size != 4)
        return false;
    
    live_sym.pop(); //界符
    Symbol sym_array = live_sym.top();//Array
    live_sym.pop();
    Symbol sym_id = live_sym.top();   //标识符
    live_sym.pop();
    Symbol sym_type = live_sym.top(); //类型
    live_sym.pop();
    
    //*更新记录数组的内存符号表*
    // 数组的每个元素都作为内存符号表的一个项进行存储；a[0][0]~a[1][1]
    // 数组的名称作为内存符号表的一个项进行存储；a*2*2
    // 通过数组名称插入时，检查是否存在数组名与变量名的重定义
    // 通过数组元素插入时，检查是否存在数组名的重定义。

    // 检查是否有数组名重定义
    auto tb_find_rlt = sym_table_manager.tbs[sym_table_manager.cur_tbidx].find(sym_id.get_name());
    if (tb_find_rlt != sym_table_manager.tbs[sym_table_manager.cur_tbidx].end()){
        printf("[Error] 出现变量名与数组名重复定义\n");
        return true;
    }
    // 解析数组的维度(Array.name)
    string array_type_str = sym_array.get_palce(); // 2 2
    stringstream convert_ss(array_type_str);
    std::vector<int> type_int;
    while(!convert_ss.eof()){
        int x;
        convert_ss >> x;
        type_int.push_back(x);
    }
    // 检查数组名id是否重定义
    if(sym_table_manager.find_item(sym_id.get_name())){
        printf("发生语义错误，变量名重定义(sym_id=%s)\n", sym_id.get_name().c_str());
        return true;
    }
    // 由 type 和 id 组成数组名称
    stringstream array_name_ss;
    array_name_ss << sym_id.get_name();
    for(auto iter = type_int.begin(); iter != type_int.end(); ++iter){
        array_name_ss << "*" << *iter;
    }
    // 检查数组名称是否重定义
    if(sym_table_manager.find_item(array_name_ss.str())){
        printf("[ERROR]发生语义错误，变量名重定义(sym_id=%s)\n", array_name_ss.str().c_str());
        return true;
    }
    // 插入数组名称符号表项
    sym_table_manager.add_item(array_name_ss.str(), sym_type.get_type());
    // 插入数组元素符号表项
    int n = type_int.size();
    stringstream array_element_name_ss;
    array_element_name_ss << sym_id.get_name();
    for(int i = 0; i < n; ++i){
        for(int j = 0; j < i; ++j){
            array_element_name_ss << "[0]";
        }
        get_all_element(type_int, i, array_element_name_ss.str(), sym_type.get_type());
    }

    //左项加入符号栈
    Symbol V_sym;
    V_sym.add_name("V");
    V_sym.add_type(sym_type.get_type());
    live_sym.push(V_sym);
    return true;
}

/* 3、检查是否为说明语句（数组）归约 -----------------
    Array  [左中括号] [数值] [右中括号] Array  Array.name = "[数值] *" + Array.name
    Array  [左中括号] [数值] [右中括号]        Array.name = "[数值]"
--------------------------------------------------*/
static bool sematype_chk_array(int GPi)
{
    int LHS_idx = GP_list_manager.GP_list[GPi].LHS; //产生式左项
    if (all_token.NT_list[LHS_idx % 10000].name != "Array")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    
    if(RHS_size == 4){
        Symbol sym_array = live_sym.top();
        live_sym.pop();
        live_sym.pop();
        Symbol sym_number = live_sym.top();
        live_sym.pop();
        live_sym.pop();

        Symbol syml_array;
        syml_array.add_name("Array");
        //stringstream ss;
        string array_place;
        array_place = sym_number.get_name() + " " + sym_array.get_palce();
        //ss << sym_number.get_name() << " " << sym_array.get_palce();
        syml_array.add_place(array_place);
        live_sym.push(syml_array);
        return true;
    }
    if(RHS_size == 3){
        live_sym.pop();
        Symbol sym_number = live_sym.top();
        live_sym.pop();
        live_sym.pop();

        Symbol syml_array;
        syml_array.add_name("Array");
        stringstream ss;
        ss << sym_number.get_name();
        syml_array.add_place(ss.str());
        live_sym.push(syml_array);
        return true;
    }
    return false;
}

/* 4、检查是否为赋值语句归约 ---------------------
    G= 	     [标识符]	[赋值号]	E	[界符]
    G= 	     [标识符]   Array	   [赋值号]	  E	 [界符]
--------------------------------------------------*/
static bool sematype_chk_fuzhi(int GPi)
{
    int LHS_idx = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    if(all_token.NT_list[LHS_idx%10000].name != "G=")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(RHS_size != 4 && RHS_size != 5)
        return false;
    
    //**赋值语句语义分析：
    //  1、检查标识符是否在内存符号表中  (语义检查)
    //  2、产生赋值四元式 (中间代码生成)
    Symbol sym_jiefu = live_sym.top();
    live_sym.pop(); //界符出栈
    Symbol sym_E = live_sym.top();
    live_sym.pop(); //E出栈
    Symbol sym_fuzhi = live_sym.top();
    live_sym.pop(); //赋值号出栈
    Symbol sym_array;
    if(RHS_size == 5){
        sym_array = live_sym.top(); 
        live_sym.pop(); //Array出栈
    }
    Symbol sym_id = live_sym.top();
    live_sym.pop(); //标识符出栈

    // 获取赋值语句左值的名称
    stringstream symname;
    symname << sym_id.get_name();
    if (RHS_size == 5)
    {
        stringstream get_array_ss;
        cout <<"test"<<endl;
        cout <<"test_get_palce:"<<sym_array.get_palce() << endl;
        get_array_ss << sym_array.get_palce();
        cout << "test:" << get_array_ss.str().c_str() << endl;
        while(!get_array_ss.eof()){
            int x;
            get_array_ss >> x;
            symname << "[" << x << "]";
        }
        printf("symname:%s\n", symname.str().c_str());
    }
    cout << "sym_array test!!!" << endl;
    if(!sym_table_manager.find_item(symname.str())){
        // 语义错误！
        printf("发生语义错误 : 赋值语句左值未声明(sym_id=%s)\n", symname.str().c_str());
        printf("error info : RHS{ %s %s %s %s }\n", 
            sym_id.get_name().c_str(), sym_fuzhi.get_name().c_str(), sym_E.get_name().c_str(), sym_jiefu.get_name().c_str());
        return true;
    }
    cout << "sym_error test!!!" << endl;
    ostringstream out;
    out << "[" << sym_table_manager.get_offset(symname.str()) << "]";
    string       E_place   = sym_E.get_palce();
    // 生成四元式
    IL_code_manager.gen(OP_fuzhi, E_place, "_", out.str());
    cout << "sym_code test!!!" << endl;
    // G= 入内存符号栈
    Symbol V_sym;
    V_sym.add_name("G=");
    cout << "sym_code test 1" << endl;
    V_sym.add_type(sym_id.get_type());
    cout << "sym_code test 2" << endl;
    live_sym.push(V_sym);
    cout << "sym_code test 3" << endl;
    return true;
}
/* 5、检查是否为表达式相关语句归约 ---------------- 
        E  A  M  N  relop Ids
--------------------------------------------------*/
static bool sematype_chk_ids(int GPi)
{
    // Ids  E
    // Ids  E  [逗号分割符] Ids
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(LHS_name != "Ids")
        return false;
    if(RHS_size == 1){ // Ids E 
        Symbol sym_rE = live_sym.top();
        live_sym.pop();
        // real_param clean && push
        std::stack<FormParam>().swap(real_param_stack);
        real_param_stack.push(FormParam{sym_rE.get_palce(), sym_rE.get_type()});
        
        Symbol sym_l;
        sym_l.add_name("Ids");
        live_sym.push(sym_l);
        return true;
    }
    if(RHS_size == 3){ // Ids E [逗号分割符] Ids 
        live_sym.pop();
        live_sym.pop();
        Symbol sym_rE = live_sym.top();
        live_sym.pop();
        real_param_stack.push(FormParam{sym_rE.get_palce(), sym_rE.get_type()});
        
        Symbol sym_l;
        sym_l.add_name("Ids");
        live_sym.push(sym_l);
        return true;
    }
    return false;
}
static bool sematype_chk_expr(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    // 1、关系运算符 relop.op = sym_op.name;
    if(LHS_name == "relop"){
        Symbol sym_op = live_sym.top();
        live_sym.pop();

        Symbol sym_relop;
        sym_relop.add_name(sym_op.get_name());
        sym_relop.add_op(sym_op.get_name());
        live_sym.push(sym_relop);
        return true;
    }
    // 2、直接传地址 L.place = R.place
    if((LHS_name == "E" && RHS_size == 1) || (LHS_name == "A" && RHS_size == 1) || (LHS_name == "M" && RHS_size == 1))
    {
        Symbol sym_r = live_sym.top();
        live_sym.pop();
        
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_place(sym_r.get_palce());
        sym_l.add_type(sym_r.get_type());
        if(LHS_name == "E"){
            sym_l.mk_tlst();
            sym_l.mk_flst();
            sym_l.up_tlst(IL_code_manager.gen_ready(OP_jnz, sym_l.get_palce(), "_", "-1"));
            sym_l.up_flst(IL_code_manager.gen_ready(OP_j  , "_", "_", "-1"));
        }
        live_sym.push(sym_l);
        return true;
    }
    if(LHS_name == "N" && RHS_size == 3){
        live_sym.pop();
        Symbol sym_r = live_sym.top();
        live_sym.pop();
        Symbol sym_id = live_sym.top();
        live_sym.pop();
        
        Symbol sym_l;
        if(sym_r.get_name() == "[左括号]"){
            //1. N   [标识符]  [左括号]  [右括号]
            ftype ft = sym_table_manager.get_ftype_func(sym_id.get_name());
            sym_l.add_name(sym_id.get_name());
            sym_l.add_type(ft.ret_type);
            sym_l.add_place(IL_code_manager.newtemp());
            live_sym.push(sym_l);
            //检查调用函数是参数表是否为空
            if(!ft.par_type.empty()){
                printf("[Error]函数不接受参数, 函数调用失败, 翻译不可信...\n");
                return false;
            }
            //生成call
            IL_code_manager.gen(OP_call, sym_id.get_name(), "_", sym_l.get_palce());
            return true;
        }else{
            //2. N  [左括号]  E  [右括号]
            sym_l.add_name(LHS_name);
            sym_l.add_place(sym_r.get_palce());
            sym_l.add_type(sym_r.get_type());
            live_sym.push(sym_l);
            return true;
        }
    }
    // 3、函数调用相关
    if(LHS_name == "N" && RHS_size == 4){
        // N [标识符] [左括号] Ids [右括号] 
        // 全局静态变量 real_param_stack
        
        live_sym.pop();
        live_sym.pop();
        live_sym.pop();
        Symbol sym_r_id = live_sym.top();
        live_sym.pop();
        //1. 检查函数名是否被声明
        if(!sym_table_manager.find_fitem(sym_r_id.get_name())){
            printf("[Error]使用未声明的函数名, 接下来的翻译不可信...\n");
            Symbol sym_l;
            sym_l.add_name("N");
            live_sym.push(sym_l);
            return false;
        }
        //1.1 实参与形参的类型匹配检查
        ftype ft = sym_table_manager.get_ftype_func(sym_r_id.get_name());
        //2. 生成param传参语句
        int index_p = 0;
        
        //**** debug
        printf("debug: ft.par_type: ");
        for(auto iter = ft.par_type.begin(); iter != ft.par_type.end(); ++iter)
            printf(" %d", *iter);
        printf("\n");
        //**** debug
        
        while(!real_param_stack.empty())
        {
            FormParam p = real_param_stack.top();
            real_param_stack.pop();
            //chk type
            printf("debug: real_param: %s (%d) \n", p.name.c_str(), p.type);
            if(p.type != ft.par_type[index_p++]){
                printf("[Error]函数参数不匹配, 函数调用失败, 翻译不可信...\n");
                Symbol sym_l;
                sym_l.add_name(sym_r_id.get_name());
                sym_l.add_type(ft.ret_type);
                live_sym.push(sym_l);
                return false;
            }
            //gen code
            IL_code_manager.gen(OP_param, p.name, "_", "_");
        }
        
        //4. N符号入栈 live_sym
        Symbol sym_lN;
        sym_lN.add_name(sym_r_id.get_name());
        sym_lN.add_type(ft.ret_type);
        sym_lN.add_place(IL_code_manager.newtemp());
        live_sym.push(sym_lN);
        // 3. 生成call函数调用语句
        IL_code_manager.gen(OP_call, sym_r_id.get_name(), "_", sym_lN.get_palce());
        return true;
    }
    // 3、传符号与传地址 L.op = Rop.name; L.place = Rn.place
    if( (RHS_size == 2) && (LHS_name == "A+" || LHS_name == "M+" || LHS_name == "N+"))
    {
        Symbol sym_rn = live_sym.top();
        live_sym.pop();
        Symbol sym_rop = live_sym.top();
        live_sym.pop();
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_place(sym_rn.get_palce());
        sym_l.add_op(sym_rop.get_name());
        live_sym.push(sym_l);
        return true;
    }
    // 4、进行简单算术运算(最终)
    if(RHS_size == 2 && (LHS_name == "A" || LHS_name == "M" ))
    {
        //出符号栈
        Symbol sym_nn = live_sym.top();
        live_sym.pop();
        Symbol sym_n = live_sym.top();
        live_sym.pop();
        //入符号栈
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_type(sym_n.get_type());
        sym_l.add_place(IL_code_manager.newtemp());
        live_sym.push(sym_l);
        //生成四元式
        string op = sym_nn.get_op();
        IL_code_manager.gen(op, sym_n.get_palce(), sym_nn.get_palce(), sym_l.get_palce());
        return true;
    }
    // 5、进行简单算术运算(中间)
    if(RHS_size == 3 && (LHS_name == "M+" || LHS_name == "N+")){
        // 出符号栈
        Symbol sym_nn = live_sym.top();
        live_sym.pop();
        Symbol sym_n  = live_sym.top();
        live_sym.pop();
        Symbol sym_op = live_sym.top();
        live_sym.pop();
        // 入符号栈
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_type(sym_n.get_type());
        sym_l.add_place(IL_code_manager.newtemp());
        sym_l.add_op(sym_op.get_name());
        live_sym.push(sym_l);
        // 生成四元式
        IL_code_manager.gen(sym_nn.get_op(), sym_n.get_palce(), sym_nn.get_palce(), sym_l.get_palce());
        return true;
    }
    // 6、标识符地址传递 or 常量值传递
    if(RHS_size == 1 && (LHS_name == "N")){
        Symbol sym_r = live_sym.top();
        live_sym.pop();
        if(sym_r.token_idx / 10000){
            // 出现未知错误
            printf("!!!\n");
            return false;
        }
        if(all_token.wr_list[sym_r.token_idx].type == "[标识符]"){
            //查找内存符号表
            bool v = sym_table_manager.find_item(sym_r.get_name());
            //查找形参表
            bool f = false;
            auto ffind = form_param_table.find(sym_r.get_name());
            if(ffind != form_param_table.end())
                f = true;
            if (!v && !f){
                printf("语义分析错误，使用未声明的变量(sym_name=%s)，注意接下来的翻译均不可信...\n", sym_r.get_name().c_str());
                Symbol sym_l;
                sym_l.add_name(LHS_name);
                live_sym.push(sym_l);
                return false;
            }
            // 右项为标识符 且 标识符在内存符号表中存在
            Symbol sym_l;
            sym_l.add_name(sym_r.get_name());
            ostringstream out;
            if(v){
                out << "[" << sym_table_manager.get_offset(sym_r.get_name()) << "]";
                sym_l.add_place(out.str());
                sym_l.add_type(sym_table_manager.get_type(sym_r.get_name()));
                live_sym.push(sym_l);
                return true;
            }
            else if(f){
                out << "[SP+" << form_param_table[sym_r.get_name()].index << "]";
                sym_l.add_place(out.str());
                sym_l.add_type(form_param_table.find(sym_r.get_name())->second.type);
                live_sym.push(sym_l);
                return true;
            }
        }else if(all_token.wr_list[sym_r.token_idx].type == "[数值]"){
            // 右项为数值常量 
            Symbol sym_l;
            sym_l.add_name(sym_r.get_name());
            sym_l.add_place(IL_code_manager.newtemp());
            live_sym.push(sym_l);
            IL_code_manager.gen(OP_fuzhi, sym_r.get_name(),"_", sym_l.get_palce());
            return true;
        }
    }
    if(RHS_size == 2 && (LHS_name == "N")){
        Symbol sym_array = live_sym.top();
        live_sym.pop();
        Symbol sym_id = live_sym.top();
        live_sym.pop();
        // 查找内存符号表
        stringstream name_ss;
        name_ss << sym_id.get_name();
        stringstream array_ss(sym_array.get_palce());
        while(!array_ss.eof()){
            int x;
            array_ss >> x;
            name_ss << "[" << x << "]";
        }
        bool v = sym_table_manager.find_item(name_ss.str());
        if(!v){
            printf("语义分析错误，使用未声明的变量(sym_name=%s)，注意接下来的翻译均不可信...\n", name_ss.str().c_str());
            Symbol sym_l;
            sym_l.add_name(LHS_name);
            live_sym.push(sym_l);
            return false;
        }
        Symbol sym_l;
        sym_l.add_name(name_ss.str());
        
        stringstream out;
        out << "[" << sym_table_manager.get_offset(name_ss.str()) << "]";
        sym_l.add_place(out.str());
        sym_l.add_type(sym_table_manager.get_type(name_ss.str()));
        live_sym.push(sym_l);
        return true;
    }
    // 7、进行关系表达式运算(最终、中间)
    if((RHS_size == 2 && LHS_name == "E") || (RHS_size == 3 && LHS_name == "A+")){
        Symbol sym_nn = live_sym.top();
        live_sym.pop();
        Symbol sym_n  = live_sym.top();
        live_sym.pop();
        
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_place(IL_code_manager.newtemp());
        //  live_sym.push(sym_l);
        
        stringstream out;
        out<<IL_code_manager.get_idx_next()+3;
        string relop_op = sym_nn.get_op();
        if(relop_op == "[算符==]" || relop_op == "=="){
            relop_op = OP_jeq;
        }else if(relop_op == "[算符!=]" || relop_op == "!="){
            relop_op = OP_jne;
        }else if(relop_op == "[算符<]"  || relop_op == "<"){
            relop_op = OP_jl;
        }else if(relop_op == "[算符<=]"  || relop_op == "<="){
            relop_op = OP_jle;
        }else if(relop_op == "[算符>]"   || relop_op == ">"){
            relop_op = OP_jg;
        }else if(relop_op == "[算符>=]"  || relop_op == ">="){
            relop_op = OP_jge;
        }else{
            // 发生未知错误
            printf("语义分析错误 : 未定义的关系算符(%s)\n", relop_op.c_str());
            return false;
        }
        IL_code_manager.gen(relop_op, sym_n.get_palce(), sym_nn.get_palce(), out.str());
        IL_code_manager.gen(OP_fuzhi, "0", "_", sym_l.get_palce());
        //out.clear();
        ostringstream out2;
        out2 << (IL_code_manager.get_idx_next() + 2);
        IL_code_manager.gen(OP_j, "_", "_", out2.str());
        IL_code_manager.gen(OP_fuzhi, "1", "_", sym_l.get_palce());

        if(LHS_name == "A+"){
            Symbol sym_relop = live_sym.top();
            live_sym.pop();
            sym_l.add_op(sym_relop.get_name());
        }else if(LHS_name == "E"){
            sym_l.mk_tlst();
            sym_l.up_tlst(IL_code_manager.gen_ready(OP_jnz, sym_l.get_palce(), "_", "-1"));
            sym_l.mk_flst();
            sym_l.up_flst(IL_code_manager.gen_ready(OP_j, "_", "_", "-1"));
        }
        live_sym.push(sym_l);
        return true;
    }
    return false;
}

/* 4、检查是否为 epsilon 语句归约 -----------------
    KM          KM.nidx = get_idx_next();
    KN          gen_ready(j,_,_,KN.nlst);
    BM          BM.nidx = get_idx_next(); BM.tbidx = sym_table_manager.place = sym_table_manager.add_tb();
--------------------------------------------------*/
static bool sematype_chk_epsilon(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();

    if(RHS_size)
        return false;

    if(LHS_name == "KM"){
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_nidx(IL_code_manager.get_idx_next());
        live_sym.push(sym_l);
        return true;
    }
    if(LHS_name == "KN"){
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.mk_nlst();
        sym_l.up_nlst(IL_code_manager.gen_ready(OP_j, "_", "_", "-1"));
        live_sym.push(sym_l);
        return true;
    }
    if(LHS_name == "BM"){
        Symbol sym_l;
        sym_l.add_name(LHS_name);
        sym_l.add_nidx(IL_code_manager.get_idx_next());
        live_sym.push(sym_l);
        int tbidx = sym_table_manager.add_tb();
        sym_table_manager.cur_tbidx = tbidx;
        stringstream in;
        in << tbidx;
        sym_l.add_place(in.str());
        return true;
    }
    return false;
}
/* 5、检查是否为 Gif 语句归约 --------------------- 
Gif 	 [关键字if]	    [左括号] E [右括号]  KM  B		
Gif 	 [关键字if]	    [左括号] E [右括号]  KM  B  KN  [关键字else] KM  B  
--------------------------------------------------*/
static bool sematype_chk_if(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(LHS_name != "Gif")
        return false;

    if(RHS_size == 6)   {
        Symbol sym_rB = live_sym.top();
        live_sym.pop(); // B
        Symbol sym_rKM = live_sym.top();
        live_sym.pop(); // KM
        live_sym.pop(); // 右括号
        Symbol sym_rE  = live_sym.top();
        live_sym.pop(); // E
        live_sym.pop(); // 左括号
        live_sym.pop(); // if

        // 1、back_patch(E.tlst, KM.nidx); 
        ostringstream out;
        out << sym_rKM.get_nidx();
        auto back_set = sym_rE.get_tlst();
        for(auto iter = back_set.begin(); iter != back_set.end(); ++iter)
            IL_code_manager.back_patch(*iter, out.str());
        // 2、Gif.nlst = merge(E.flst, B.nlst);
        Symbol sym_lGif;
        sym_lGif.add_name(LHS_name);
        sym_lGif.mk_nlst();
        auto E_flst = sym_rE.get_flst();
        auto B_nlst = sym_rB.get_nlst();
        for(auto iter = E_flst.begin(); iter != E_flst.end(); ++iter)
            sym_lGif.up_nlst(*iter);
        for(auto iter = B_nlst.begin(); iter != B_nlst.end(); ++iter)
            sym_lGif.up_nlst(*iter);

        live_sym.push(sym_lGif);
        return true;
    }
    if(RHS_size == 10)  {
        Symbol sym_rB2 = live_sym.top();
        live_sym.pop(); //B2
        Symbol sym_rKM2 = live_sym.top();
        live_sym.pop(); // KM2
        live_sym.pop(); // else
        Symbol sym_rKN = live_sym.top();
        live_sym.pop(); // KN
        Symbol sym_rB1  = live_sym.top();
        live_sym.pop(); // B1
        Symbol sym_rKM1 = live_sym.top();
        live_sym.pop(); // KM1
        live_sym.pop(); // 右括号
        Symbol sym_rE = live_sym.top();
        live_sym.pop(); // E
        live_sym.pop(); // 左括号
        live_sym.pop(); // if

        // back_patch(E.tlst, KM1.nidx); 
        ostringstream out;
        out << sym_rKM1.get_nidx();
        auto E_tlst = sym_rE.get_tlst();
        for(auto iter = E_tlst.begin(); iter != E_tlst.end(); ++iter)
            IL_code_manager.back_patch(*iter, out.str());
        // back_patch(E.flst, KM2.nidx);
        out.clear();
        ostringstream out2;
        out2 << sym_rKM2.get_nidx();
        auto E_flst = sym_rE.get_flst();
        for(auto iter = E_flst.begin(); iter != E_flst.end(); ++iter)
            IL_code_manager.back_patch(*iter, out2.str());
        
        // Gif.nlst = merge(B1.nlst, KN.nlst, B2.nlst);        
        Symbol sym_lGif;
        sym_lGif.add_name(LHS_name);
        sym_lGif.mk_nlst();
        auto B1_nlst = sym_rB1.get_nlst();
        auto KN_nlst = sym_rKN.get_nlst();
        auto B2_nlst = sym_rB2.get_nlst();
        for(auto iter = B1_nlst.begin(); iter != B1_nlst.end(); ++iter)
            sym_lGif.up_nlst(*iter);
        for(auto iter = KN_nlst.begin(); iter != KN_nlst.end(); ++iter)
            sym_lGif.up_nlst(*iter);
        for(auto iter = B2_nlst.begin(); iter != B2_nlst.end(); ++iter)
            sym_lGif.up_nlst(*iter);
        
        live_sym.push(sym_lGif);
        return true;
    }
    return true;
}
/* 6、检查是否为 Gwhile 语句归约 ------------------
Gwhile 	 [关键字while]  	[左括号]	 KM  E  [右括号]  KM  B 
--------------------------------------------------*/
static bool sematype_chk_while(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;  //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();

    if(LHS_name != "Gwhile")
        return false;
    
    if(RHS_size == 7){
        Symbol sym_rB = live_sym.top();
        live_sym.pop(); //B
        Symbol sym_rKM2 = live_sym.top();
        live_sym.pop(); //KM2
        live_sym.pop(); //右括号
        Symbol sym_rE = live_sym.top();
        live_sym.pop(); //E
        Symbol sym_rKM1 = live_sym.top();
        live_sym.pop(); //KM1
        live_sym.pop(); //左括号
        live_sym.pop(); //while

        //back_patch(E.tlst, KM2.nidx);
        ostringstream out;
        auto E_tlst = sym_rE.get_tlst();
        out << sym_rKM2.get_nidx();
        for(auto iter = E_tlst.begin(); iter != E_tlst.end(); ++iter)
            IL_code_manager.back_patch(*iter, out.str());
        //back_patch(B.nlst, KM1.nidx);
        auto B_nlst = sym_rB.get_nlst();
        //out.clear();
        ostringstream out2;
        out2 << sym_rKM1.get_nidx();
        for(auto iter = B_nlst.begin(); iter != B_nlst.end(); ++iter)
            IL_code_manager.back_patch(*iter, out2.str());
        //Gwhile.nlst = E.flst;
        Symbol sym_lGwhile;
        sym_lGwhile.add_name(LHS_name);
        sym_lGwhile.mk_nlst();
        auto E_flst = sym_rE.get_flst();
        for(auto iter = E_flst.begin(); iter != E_flst.end(); ++iter)
            sym_lGwhile.up_nlst(*iter);
        // gen(j, _, _, KM1.nidx);
        IL_code_manager.gen(OP_j, "_", "_", out2.str());
        live_sym.push(sym_lGwhile);
        return true;
    }
    return false;
}
/* 6、检查是否为 Greturn 语句归约 ------------------
Greturn  [关键字return]	E	[界符]  
--------------------------------------------------*/
static bool sematype_chk_ret(int GPi)
{
    int LHS_idx = GP_list_manager.GP_list[GPi].LHS; //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx % 10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if (LHS_name != "Greturn" || RHS_size != 3)
        return false;

    live_sym.pop();
    Symbol sym_rE = live_sym.top();
    live_sym.pop();
    live_sym.pop();

    Symbol sym_lGret;
    sym_lGret.add_name(LHS_name);
    live_sym.push(sym_lGret);

    IL_code_manager.gen(OP_ret, sym_rE.get_palce(), "_", "_");

    return true;
}
/* 7、检查是否为 G 语句归约 ----------------------- 
    G 	  G=|Greture|Gwhile|Gif
--------------------------------------------------*/
static bool sematype_chk_G(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(LHS_name != "G" || RHS_size != 1)
        return false;
    
    int RHS_vidx = GP_list_manager.GP_list[GPi].RHS[0]; //产生式右项
    string RHS_name = all_token.NT_list[RHS_vidx % 10000].name;

    // 开始语义分析
    Symbol sym_rG = live_sym.top();
    live_sym.pop();
    Symbol sym_lG;
    sym_lG.add_name(LHS_name);
    sym_lG.mk_nlst();
    if(RHS_name == "Gif" || RHS_name == "Gwhile"){
        auto rG_nlst = sym_rG.get_nlst();
        for(auto iter = rG_nlst.begin(); iter != rG_nlst.end(); ++iter)
            sym_lG.up_nlst(*iter);
    }
    live_sym.push(sym_lG);
    return true;
}
/* 8、检查是否为 B2 语句归约 --------------------- 
    B2 	     G				
    B2 	     G	KM B2
--------------------------------------------------*/
static bool sematype_chk_B2(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(LHS_name != "B2")
        return false;

    if(RHS_size == 1){
        Symbol sym_rG = live_sym.top();
        live_sym.pop();
        Symbol sym_lB2;
        sym_lB2.add_name(LHS_name);
        sym_lB2.mk_nlst();
        auto G_nlst = sym_rG.get_nlst();
        for(auto iter = G_nlst.begin(); iter != G_nlst.end(); ++iter)
            sym_lB2.up_nlst(*iter); 
        live_sym.push(sym_lB2);
        return true;
    }
    if(RHS_size == 3){
        Symbol sym_rB2 = live_sym.top();
        live_sym.pop();
        Symbol sym_rKM = live_sym.top();
        live_sym.pop();
        Symbol sym_rG  = live_sym.top();
        live_sym.pop();
        // back_patch(G.nlst, KM.nidx);
        auto G_nlst = sym_rG.get_nlst();
        ostringstream out;
        out << sym_rKM.get_nidx();
        for(auto iter = G_nlst.begin(); iter != G_nlst.end(); ++iter)
            IL_code_manager.back_patch(*iter, out.str());
        // B2.nlst = B2'.nlst;
        Symbol sym_lB2;
        sym_lB2.add_name(LHS_name);
        sym_lB2.mk_nlst();
        auto rB2_nlst = sym_rB2.get_nlst();
        for(auto iter = rB2_nlst.begin(); iter != rB2_nlst.end(); ++iter)
            sym_lB2.up_nlst(*iter);
        
        live_sym.push(sym_lB2);
        return true;
    }
    return false;
}
/* 9、检查是否为 B 语句归约 ----------------------- 
    B	     [左大括号]	B1	B2	[右大括号]
    B 	     [左大括号]	B2	[右大括号]	
--------------------------------------------------*/
static bool sematype_chk_B(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(LHS_name != "B")
        return false;

    live_sym.pop();
    Symbol sym_rB2 = live_sym.top();
    live_sym.pop();
    live_sym.pop();
    if(RHS_size == 4) 
        live_sym.pop();

    Symbol sym_lB;
    sym_lB.add_name(LHS_name);
    sym_lB.mk_nlst();
    auto B2_nlst = sym_rB2.get_nlst();
    for(auto iter = B2_nlst.begin(); iter != B2_nlst.end(); ++iter)
        sym_lB.up_nlst(*iter);
    
    live_sym.push(sym_lB);
    return true;
}
/* 10、检查是否为 F 语句归约 ---------------------------- 
    F        T  [标识符]    [左括号]  BM  [右括号]  B	
    F        T  [标识符]    [左括号]  Args  AM BM  [右括号]  B
--------------------------------------------------------*/
static bool sematype_chk_F(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    if(LHS_name!="F")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    
    // 从BM中取得 nidx 和 tbidx
    unsigned int nidx  =  0;
    int          tbidx = -1;
    for(int i = 0; i < RHS_size-2; ++i){
        if(i == 2){
            Symbol BM = live_sym.top();
            nidx = BM.get_nidx();
            tbidx = atoi(BM.get_palce().c_str());
        }
        live_sym.pop();
    }
    Symbol sym_id_r = live_sym.top(); //得到产生右项的标识符
    live_sym.pop();
    Symbol sym_ret_type_r = live_sym.top(); //得到产生式右项的类型
    live_sym.pop();
    
    //将函数名放入全局符号表中
    ftype ft;
    ft.ret_type = sym_ret_type_r.get_type();
    ft.par_type.resize(form_param_table.size());
    for(auto iter = form_param_table.begin(); iter != form_param_table.end(); ++iter){
        ft.par_type[iter->second.index] = iter->second.type;
    }
    sym_table_manager.add_fitem(sym_id_r.get_name(), nidx, ft, tbidx);
    printf("debug: add item to Function table : %s\n", sym_id_r.get_name().c_str());
    Symbol sym_l;
    sym_l.add_name(sym_id_r.get_name());
    sym_l.add_type(ft.ret_type);
    live_sym.push(sym_l);

    //清空形参相关记录，以便下一个函数声明被正确翻译
    std::map<std::string, FormParamTableItem>().swap(form_param_table);
    std::stack<FormParam>().swap(form_param_stack);
    
    return true;
}
static bool sematype_chk_AM(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    if(LHS_name != "AM")
        return false;
    // 通过 form_param_stack 构造 form_param_table
    int p_index = 0;
    while(!form_param_stack.empty())
    {
        FormParam t = form_param_stack.top();
        form_param_stack.pop();
        form_param_table[t.name] = (FormParamTableItem){p_index++, t.type};
    }
    Symbol sym_l;
    sym_l.add_name("AM");
    live_sym.push(sym_l);
    return true;
}
/* 11、检查是否为 Args 语句归约 ------------------- 
    Args     T  [标识符]   	
    Args     T  [标识符]  [逗号分割符]  Args		
--------------------------------------------------*/
static bool sematype_chk_Args(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    if(LHS_name != "Args")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(RHS_size != 2 && RHS_size != 4)
        return false;    
    
    if(RHS_size == 4){
        live_sym.pop();
        live_sym.pop();
    }

    Symbol sym_r_id = live_sym.top();
    live_sym.pop();
    Symbol sym_r_type = live_sym.top();
    live_sym.pop();

    //加入form_param_stack
    form_param_stack.push(FormParam{sym_r_id.get_name(), sym_r_type.get_type()});
    
    Symbol sym_l;
    sym_l.add_name("Args");
    live_sym.push(sym_l);
    return true;
}
/* 12、检查是否为 T 语句归约 ---------------------- 
    T        [关键字int]	  	
    T        [关键字void]	  	
    T        [关键字float]	  	
--------------------------------------------------*/
static bool sematype_chk_T(int GPi)
{
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    if(LHS_name!="T")
        return false;
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    if(RHS_size != 1)
        return false;
    Symbol sym_r = live_sym.top(); //得到产生式右项的符号
    live_sym.pop();
    
    Symbol sym_lT;
    sym_lT.add_name("T");
    sym_lT.add_type(sym_r.get_type());
    live_sym.push(sym_lT);
    
    return true;
}
//** 归约时语义检查、中间代码生成入口函数
static bool sema_check(int GPi /*归约时使用的产生式序号*/)
{
    printf("sema_check: GPi = %d\n", GPi);
    //不同语法产生式对应不同的语义分析//
    if(sematype_chk_shuoming(GPi)){ //说明语句
        printf("test : finish sematype_chk_shuoming (GPi=%d)\n", GPi);
        // debug_print_MemSymTable();
        return true;
    }
    if (sematype_chk_shuoming_array(GPi)){ //数组说明语句
        printf("test : finish sematype_chk_shuoming_array (GPi=%d)\n", GPi);
        // debug_print_MemSymTable();
        return true;
    }
    if (sematype_chk_array(GPi)){ //数组下标
        printf("test : finish sematype_chk_array (GPi=%d)\n", GPi);
        // debug_print_MemSymTable();
        return true;
    }
    if(sematype_chk_fuzhi(GPi)){
        printf("test : finish sematype_chk_fuzhi (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_Args(GPi)){
        printf("test : finish sematype_chk_Args (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_AM(GPi)){
        printf("test : finish sematype_chk_AM (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_F(GPi)){
        printf("test : finish sematype_chk_F (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_ids(GPi)){
        printf("test : finish sematype_chk_ids (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_expr(GPi)){
        printf("test : finish sematype_chk_expr (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_epsilon(GPi)){
        printf("test : finish sematype_chk_epsilon (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_if(GPi)){
        printf("test : finish sematype_chk_if (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_while(GPi)){
        printf("test : finish sematype_chk_while (GPi=%d)\n", GPi);
        return true;
    }
    if (sematype_chk_ret(GPi))
    {
        printf("test : finish sematype_chk_ret (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_G(GPi)){
        printf("test : finish sematype_chk_G (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_B2(GPi)){
        printf("test : finish sematype_chk_B2 (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_B(GPi)){
        printf("test : finish sematype_chk_B (GPi=%d)\n", GPi);
        return true;
    }
    if(sematype_chk_T(GPi)){
        printf("test : finish sematype_chk_T (GPi=%d)\n", GPi);
        return true;
    }
    // 暂无语义子程序需要运行的产生式归约
    int RHS_size = GP_list_manager.GP_list[GPi].RHS.size();
    for(int i = 0; i < RHS_size; ++i){
        live_sym.pop();
    }
    int LHS_idx  = GP_list_manager.GP_list[GPi].LHS;    //产生式左项
    string LHS_name = all_token.NT_list[LHS_idx%10000].name;
    Symbol sym_l;
    sym_l.add_name(LHS_name);
    live_sym.push(sym_l);
    return false;
}

void debug_print_live_sym()
{
    // for debug;
    stack<Symbol> tmp_live_sym(live_sym);
    printf("[ live_sym (%d) ] ", tmp_live_sym.size());
    while(!tmp_live_sym.empty()){
        printf(" %s", tmp_live_sym.top().get_name().c_str());
        tmp_live_sym.pop();
    }
    printf("\n");
}

void debug_print_live_state()
{
    stack<int> tmp_live_state(live_state);
    printf("[ live_state (%d) ] ", tmp_live_state.size());
    while(!tmp_live_state.empty()){
        printf(" %d", tmp_live_state.top());
        tmp_live_state.pop();
    }
    printf("\n");
}

// 词法终结符进入符号栈
// 1、语义属性设置 2、进入符号栈
static void push_live_sym(WInfoItem & winfo)
{
    // 构造内存符号
    Symbol w_symbol;
    w_symbol.token_idx = winfo.index;
    w_symbol.add_name(winfo.word);
    if(winfo.type == "[关键字int]"){
        vtype t = type_int;
        w_symbol.add_type(t);
    }else if(winfo.type == "[关键字float]"){
        vtype t = type_float;
        w_symbol.add_type(t);
    }else if(winfo.type == "[关键字void]"){
        vtype t = type_void;
        w_symbol.add_type(t);
    }

    // 内存符号栈移进
    live_sym.push(w_symbol);

    // for debug;
    printf("[ live_sym <- %s ]", w_symbol.get_name().c_str());
    stack<Symbol> tmp_live_sym(live_sym);
    while(!tmp_live_sym.empty()){
        printf(" %s", tmp_live_sym.top().get_name().c_str());
        tmp_live_sym.pop();
    }
    printf("\n");
}

//:语法分析驱动程序:
// ps. 这里为啥叫 lalr 呢？因为一开始有伟大的构想！但是现实...只能是lr1...
bool lalr_ini(const string gplst_fpath)
{
    INFO lr_info;
 //符号表初始化
    all_token.NT_list = NT_list;
    all_token.wr_list = myclex->get_wr_list();
 //建立文法产生式表
    if(!GP_list_manager.ini(gplst_fpath, &all_token)){
        lr_info.tolog(_LOG_T_ERROR, "lr1 : 文法产生式表建立失败.");
        return false;
    }
    lr_info.tolog(_LOG_T_INFO, "lr1 : 文法产生式表建立成功.");
    GP_list_manager.debug_print();
 //生成所有非终结符的FIRST集合
    ini_all_nt_first_set();
 //建立项集族
 //构建LR1语法分析表
    if(!get_iterms_list()){
        lr_info.tolog(_LOG_T_ERROR, "lr1 : 语法分析表建立失败.");
        return false;
    };
    lr_info.tolog(_LOG_T_INFO, "lr1 : 语法分析表建立成功.");

    debug_print_iterms_list();
    debug_print_Action_Goto_Table();
    lr_info.tolog(_LOG_T_INFO, "lr1 : 语法分析器初始化完成.");
    return true;
}
//:lr1分析整个流程
bool lalr_run(string file_path, string gplst_fpath = GPLIST_PATH)
{
    INFO lr_info;
    fstream fout_stack_info;
    //打开记录栈变化的文件
    fout_stack_info.open(STACK_PATH, std::ios::out);
    if(!fout_stack_info.is_open()){
        string info1 = "lr1 : 栈情况记录失败，文件打开失败(path=";
        string path  = STACK_PATH;
        string info2 = ").";
        lr_info.tolog(_LOG_T_ERROR, info1+path+info2);
    }
    //设置语法分析树的绘制状态（true表示绘制 false表示不绘制）
    bool dot_status = true; 
    lr_info.tolog(_LOG_T_INFO, string("当前语法分析树的绘制状态为:") + string((dot_status? "True":"False")));
    
    //1.词法分析器初始化
    CLEX clex = CLEX(file_path);
    myclex = &clex;
    myclex->predo();
    //2.语法分析器初始化
    lalr_ini(gplst_fpath);
    //3.开始语法分析
    stack<int>().swap(live_state); //清空 live_state (活状态栈)
    stack<int>().swap(live_dot);   //清空 live_dot (用于绘制语法树)
    stack<Symbol>().swap(live_sym);//情况 live_sym (活内存符号栈)
    dot_count = 0;
    live_state.push(0); //移入状态0
    //** 1.调用词法分析器，取得当前词汇
    WInfoItem winfo = myclex->getword();
    if(winfo.index == -1){
        //tolog : 词法分析错误
        lr_info.tolog(_LOG_T_ERROR, "lr1 : 由于词法分析错误，语法分析中断退出.");
        printf("由于词法分析错误，语法分析中断退出。\n");
        return false;
    }
    if (dot_status) //初始化语法树绘制文件
        dot_status = graph_to_file_ini();
    //** 2.根据ActionGoto表，进行动作处理
    while(1) { 
        auto find_action = Action_Goto_Table[live_state.top()].action_map.find(winfo.index);
        //tolog : 语法错误
        if(find_action ==  Action_Goto_Table[live_state.top()].action_map.end()){
            ostringstream out;
            out << "lr1 : 语法错误，语法分析中断退出 ( word_type = " << winfo.type << " ).";
            lr_info.tolog(_LOG_T_ERROR, out.str());
            printf("语法错误，语法分析中断退出。(word_type = %s)\n", winfo.type.c_str());
            return false;
        }
        //移进
        if(find_action->second.action_type == ACTION_T_S){       //移进
            // 状态栈移进
            live_state.push(find_action->second.nx_id);
            // 顶点栈移进
            if(dot_status)
                live_dot.push(dot_count++);
            // 符号栈移进
            push_live_sym(winfo);
            // 打印栈信息
            if(fout_stack_info.is_open()){
                fout_stack_info << "[入栈 s"<<find_action->second.nx_id<<"] live_state <- " << find_action->second.nx_id << "(" <<winfo.type.c_str()<< ")\n";
                fout_stack_info.flush();
            }
            winfo = myclex->getword();
            if(winfo.index == -1){
                //tolog : 词法分析错误
                lr_info.tolog(_LOG_T_ERROR, "lr1 : 由于词法分析错误，语法分析中断退出.");
                printf("由于词法分析错误，语法分析中断退出。\n");
                return false;
            }
        }else if(find_action->second.action_type == ACTION_T_R){ //归约
            //检查是否为 r0，是则语法分析结束（r0 <==> acc）
            if(find_action->second.nx_id == 0){
                break; //acc
            }
            auto valid_GP = GP_list_manager.GP_list[find_action->second.nx_id];
            //右部对应状态出栈
            int i;
            for(i = 0; i < (int)(valid_GP.RHS.size()); ++i){
                if(live_state.empty())
                    break;
                if(fout_stack_info.is_open()){
                    fout_stack_info << "[出栈 r" << find_action->second.nx_id <<"] live_state -> " << live_state.top() << "\n";
                    fout_stack_info.flush();
                }
                printf("debug: live_state -> %d (r%d)\n", live_state.top(), find_action->second.nx_id);
                live_state.pop();
            }
            if(i!=(int)(valid_GP.RHS.size())){
                //tolog : 语法分析归约步骤发生未知错误
                ostringstream out;
                out << "lr1 : 语法分析归约未知错误 (GPi = "<< find_action->second.nx_id << ").";
                lr_info.tolog(_LOG_T_ERROR, out.str());
                printf("语法分析归约未知错误。(GPi = %d)\n", find_action->second.nx_id);
                return false;
            }
            //左部对应状态入栈
            auto find_goto = Action_Goto_Table[live_state.top()].goto_map.find(valid_GP.LHS);
            if(find_goto == Action_Goto_Table[live_state.top()].goto_map.end()){
                //tolog : 语法错误
                ostringstream out;
                out << "lr1 : 语法错误，语法分析中断退出. (goto表项不存在 I=" <<live_state.top() << " nt_vid=" << valid_GP.LHS << ")";
                lr_info.tolog(_LOG_T_ERROR, out.str()); 
                printf("语法错误，语法分析中断退出。(goto表项不存在 I=%d nt_vid=%d)\n", live_state.top(), valid_GP.LHS);
                return false;
            }
            if(fout_stack_info.is_open()){
                fout_stack_info << "[入栈 r"<<find_action->second.nx_id<<"] live_state <- " << find_goto->second << "\n";
                fout_stack_info.flush();
            }
            printf("debug: live_state <- %d (r%d)\n", find_goto->second, find_action->second.nx_id);
            live_state.push(find_goto->second);

            //归约时符号栈变化、语义检查、中间代码生成
            sema_check(find_action->second.nx_id);
            fflush(stdout); //及时刷新缓冲区
            debug_print_live_sym();
            //归约时画图
            if(dot_status)
                dot_status = graph_to_file(find_action->second.nx_id);

        }else{
            //tolog
            lr_info.tolog(_LOG_T_ERROR, "lr1 : 未知错误，语法分析表中的action_type不是合法值.");
            printf("未知错误，语法分析表中的action_type不是合法值!\n");
            return false;
        }
        debug_print_live_state();
    }
    if(dot_status) //结束语法分析树绘制
        dot_status = graph_to_file_end();
    fstream fout_code_info;
    fout_code_info.open(ILCODE_PAHT, std::ios::out);
    sym_table_manager.table_to_file(fout_code_info);
    IL_code_manager.code_to_file(fout_code_info);
    fout_code_info.close();
    lr_info.tolog(_LOG_T_INFO, "lr1 : 语法分析成功结束.");
    return true;
}
/*
int main()
{
    if(lalr_run(SOURCE_PATH)){
        printf("[success] 语法分析成功!\n"); 
    }else{
        printf("[fail   ] 语法分析失败.\n"); 
    }
    return 0;
}
*/
// to QT interface
string get_token_lst()
{
    ostringstream out;
    for(int i = 0; all_token.wr_list[i].type != ""; ++i){
        out << i <<"\t\t" << all_token.wr_list[i].type << "\n";
    }
    for(int i = 0; all_token.NT_list[i].name != ""; ++i){
        out << all_token.NT_list[i].name << "\t\t" << all_token.NT_list[i].desc << "\n";
    }
    return out.str();
}

string get_GP_lst()
{
    ostringstream out;
    for(auto iter = GP_list_manager.GP_list.begin(); iter != GP_list_manager.GP_list.end(); ++iter){
        out << all_token.NT_list[iter->LHS%10000].name << "->";
        for(auto r_iter = iter->RHS.begin(); r_iter != iter->RHS.end(); ++r_iter){
            if(*r_iter / 10000){
                out <<" "<< all_token.NT_list[*r_iter%10000].name;
            }else{
                out <<" "<< all_token.wr_list[*r_iter].type;
            }
        }
        out << "\n";
    }
    return out.str();
}

string get_wr_info(int & wr_num)
{
    int i;
    ostringstream out;
    for(i=0; all_token.wr_list[i].type!=""; ++i){
        out << all_token.wr_list[i].type << "\n";
    }
    wr_num = i;
    return out.str();
}

string get_NT_info(int & NT_num)
{
    int i;
    ostringstream out;
    for(i=0; all_token.NT_list[i].name!=""; ++i){
        out << all_token.NT_list[i].name << "\n";
    }
    NT_num = i;
    return out.str();
}

TableIterm get_ActionGoto(int TI_index)
{
    return Action_Goto_Table[TI_index];
}
const int get_ActionGoto_size()
{
    return Action_Goto_Table.size();
}

