#pragma once
#include "common.h"

/*
	类：语法分析 - Parser
*/
class Parser
{
public:
	Parser();
	~Parser();

	/*
		功能1：加载变元映射表
		return:	map<string, string>
	*/
	void load_argument_def();

	/*
		功能2：加载初始文法（已扩展）
		return:	vector<production>
	*/
	void load_production_raw();
	void show_production_raw();

	/*
		功能3：获取单个变元的FIRST集
		return: map<string, vector<string>>
	*/
	std::vector<std::string> get_single_first(std::string argument);

	/*
		功能4：获取所有变元的FIRST集
		return: map<string, vector<string>>
	*/
	void get_first();
	void show_first();

	/*
		功能5：获取一个项目的闭包
		return: 该项目的闭包构成的项目集
	*/
	itemGroup get_single_closer(item im);
	void show_closure();

	/*
		功能6：获取 get_closer 的输入 - get_item_group seris 1
		input: 一个项目集
		return: 一个<待生成闭包的项目>的集合
	*/
	std::vector<itemGroup> get_GOTO_input(const itemGroup& in_ig);
	void show_GOTO_input(std::vector<itemGroup> ig_v);

	/*
		功能7：获取项目集的 closer - get_item_group seris 2
		return: I_k - 一个属于规范族的项目集
	*/
	itemGroup get_closer(const itemGroup& in_ig);

	/*
		功能8：获取项目集规范族，结果存储在 std::set<itemGroup> Item_group_set;
	*/
	void get_item_group();
	void show_item_group();
	void show_item_group_shift_mp();

	/*
		功能9：获取LR1分析表，结果存储在 LR1_table
	*/
	void get_LR1_table();
	void show_LR1_table();

	/*
		功能10：使用LR1分析表进行分析
		return: 分析是否成功
	*/
	bool Analyse(std::vector<token> tk);

private:
	// 1.变元映射表（仅名称，一一对应）
	std::map<std::string, std::string> Grammar_symbol_mp;
	// 2."变元-文法"映射表 - 该变元为左部的所有产生式索引，在 load_production_raw 加载
	//	  				   - 索引的是 Grammar_symbol_mp[...] 映射后的名称
	std::map<std::string, std::vector<int>> Argument_garmmar_mp;
	// 3.变元类型索引
	// Tips：判断 X 是否是终结符 -> !Argument_type.count(X) || Argument_type[X] == TERMINAL
	std::unordered_map<std::string, int> Argument_type;
	// 4.初始文法（已扩展）
	std::vector<production> Production_raw;
	// 5.所有变元的FIRST集
	std::map<std::string, std::vector<std::string>> First_set;

	// 项目集族
	// 1.项目集个数
	int Item_group_num;
	// 2.[I_i-(s)->I_j] - 状态I_i点后为s，转移到状态I_j - 索引表
	std::vector<std::map<std::string,int>> Item_group_shift_mp;
	// 3.项目集族set
	std::set<itemGroup> Item_group_set;
	// 4.产生式规约ACTION构造的依据（点在产生式右部的最后） - 行数(size)=产生式的个数
	// 记录的格式：Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,a]=j
	std::vector<std::map<std::string, std::vector<int>>> Prod_acc_mp;

	// LR(1)分析表
	// ACTION和GOTO通过规则区分
	// 规则：pair<s(移进)/r(规约)/a(accept)/g(goto), int(next state)>
	std::vector< std::map<std::string, std::pair<char, int>> > LR1_table;
};

