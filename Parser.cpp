#include "tool.h"
#include "parser.h"

/* BEGIN - class Parser - 函数实现 */
// 构造
Parser::Parser()
{
	this->Item_group_num = 0;
}

// 析构
Parser::~Parser()
{
}

// 变元映射表
void Parser::load_argument_def()
{
	for (int i = 0; i < Grammar_symbol_NUM; i++) {
		Grammar_symbol_mp[Grammar_symbol_name[i]] = Grammar_symbol_def[i];
		if (Grammar_symbol_name[i] != "ID" && Grammar_symbol_name[i] != "num" && Grammar_symbol_name[i] != "epsilon")
			this->Argument_type[Grammar_symbol_def[i]] = ARGUMENT;
		else
			this->Argument_type[Grammar_symbol_def[i]] = TERMINAL;
	}
}

// 初始文法
void Parser::load_production_raw()
{
	// 扩展
	Production_raw.push_back(production(0, ARGUMENT, -1, (std::string)("Start"), std::vector<std::string>{Grammar_symbol_mp[Grammar_rule[0][0]]}));
	this->Argument_garmmar_mp[(std::string)("Start")].push_back((int)Production_raw.size() - 1);
	// 其他
	for (int i = 0; i < Grammar_rule_NUM; ++i) {
		production prod_tmp;
		prod_tmp.id = i + 1;
		if (Grammar_rule[i][0] == "ID" || Grammar_rule[i][0] == "num" || Grammar_rule[i][0] == "epsilon")
			prod_tmp.type = TERMINAL;
		else
			prod_tmp.type = ARGUMENT;
		prod_tmp.left = Grammar_symbol_mp[Grammar_rule[i][0]];
		for (int j = 1; j < Grammar_rule[i].size(); ++j) {
			if (Grammar_symbol_mp.count(Grammar_rule[i][j]))
				prod_tmp.right.push_back(Grammar_symbol_mp[Grammar_rule[i][j]]);
			else
				prod_tmp.right.push_back(Grammar_rule[i][j]);
		}
		Production_raw.push_back(prod_tmp);
		if(prod_tmp.type == ARGUMENT)
			Argument_garmmar_mp[prod_tmp.left].push_back((int)Production_raw.size() - 1);
	}
	// for LR1_table
	Prod_acc_mp.resize(Production_raw.size());
}

// 打印初始文法（调试）
void Parser::show_production_raw()
{
	std::ofstream outfile;
	outfile.open(PS_Production_Path, std::ios::out | std::ios::binary);
	for (int i = 0; i < Production_raw.size(); ++i) {
		outfile << i << ":" << Production_raw[i].left << "->";
		for (int j = 0; j < Production_raw[i].right.size(); ++j) {
			outfile << Production_raw[i].right[j];
		}
		outfile << "\n";
	}
	outfile.close();
}

// 获取单个变元的FIRST集
std::vector<std::string> Parser::get_single_first(std::string argument)
{
	std::vector<std::string> ret_first_set;
	// 遍历所有 argument 为 left 的产生式
	for (int idx = 0; idx < Argument_garmmar_mp[argument].size(); ++idx) {
		int i = Argument_garmmar_mp[argument][idx]; // 该变元的所有产生式索引
		// Production_raw[i].right[0] 本身是终结符
		if (!Argument_type.count(Production_raw[i].right[0]) || Production_raw[i].right[0] == Grammar_symbol_mp["ID"] 
			|| Production_raw[i].right[0] == Grammar_symbol_mp["num"] || Production_raw[i].right[0] == Grammar_symbol_mp["epsilon"]) {
			if(!is_string_in_vector(ret_first_set, Production_raw[i].right[0]))
				ret_first_set.push_back(Production_raw[i].right[0]);
		}
		// Production_raw[i].right[0] 本身是变元
		else {
			// 向后看，直到【1.某个终结符】或【2.某个变元的FIRST不含epsilon】
			for (int j = 0; j < Production_raw[i].right.size(); ++j) {
				// 如果是终结符
				if (!Argument_type.count(Production_raw[i].right[j]) || Production_raw[i].right[j] == Grammar_symbol_mp["ID"]
					|| Production_raw[i].right[j] == Grammar_symbol_mp["num"] || Production_raw[i].right[j] == Grammar_symbol_mp["epsilon"]) {
					if (!is_string_in_vector(ret_first_set, Production_raw[i].right[j]))
						ret_first_set.push_back(Production_raw[i].right[j]);
					break; //1.
				}
				// 如果是变元
				else {
					std::vector<std::string> tmp_first_set;
					// 该变元已经求过FIRST
					if (First_set.count(Production_raw[i].right[j]))
						tmp_first_set = First_set[Production_raw[i].right[j]];
					else {
						// 递归
						tmp_first_set = get_single_first(Production_raw[i].right[j]);
						First_set[Production_raw[i].right[j]] = tmp_first_set;
					}
					// 补充FIRST
					for (int k = 0; k < tmp_first_set.size(); ++k) {
						if(!is_string_in_vector(ret_first_set, tmp_first_set[k]))
							ret_first_set.push_back(tmp_first_set[k]);
					}
					if (!is_string_in_vector(tmp_first_set, Grammar_symbol_mp["epsilon"]))
						break; //2.
				}
			}
		}
	}
	return ret_first_set;
}

// 获取所有变元的FIRST集
void Parser::get_first()
{
	for (int i = 0; i < this->Production_raw.size(); ++i) {
		// 产生式 raw 的 left 必然是变元
		if (First_set.count(Production_raw[i].left))
			continue;// 已经求过
		First_set[Production_raw[i].left] = get_single_first(Production_raw[i].left);
	}
}

// 打印所有变元的FIRST集
void Parser::show_first()
{
	for (auto it = First_set.begin(); it != First_set.end(); ++it) {
		std::cout << (*it).first << "{";
		for (int i = 0; i < (*it).second.size(); ++i) {
			std::cout << (*it).second[i] << ",";
		}
		std::cout << "}\n";
	}
}

// 获取一个项目的闭包（产生的 item 中 dot的位置都是在 right 的开头）
itemGroup Parser::get_single_closer(item im)
{
	itemGroup ret_ig;
	// 点的位置在末尾
	if (im.prod.dot_pos >= im.prod.right.size()) {
		ret_ig.item_group.insert(im);
		return ret_ig;
	}
	std::queue<item> qwait; // 等待队列
	qwait.push(im);
	while (qwait.size()) {
		item im_tmp = qwait.front();
		// 添加到 ret_ig
		ret_ig.item_group.insert(im_tmp);
		qwait.pop();
		production prod_tmp = im_tmp.prod;
		std::string lookforward_tmp = im_tmp.lookforward;
		// 情形1：点在产生式的末尾，则后续不入队
		if (prod_tmp.dot_pos >= prod_tmp.right.size()) {
			continue;
		}
		// 情形2：点后为终结符，则后续不入队
		if (!Argument_type.count(prod_tmp.right[prod_tmp.dot_pos]) || Argument_type[prod_tmp.right[prod_tmp.dot_pos]] == TERMINAL) {
			continue;
		}
		// 情形3：点后为变元
		// 需要点后移后的"剩余序列first集"
		std::vector<std::string> seqence_first;
		// 一般
		for (int j = prod_tmp.dot_pos + 1; j < prod_tmp.right.size(); ++j) {
			// 非终结符
			if (!Argument_type.count(prod_tmp.right[j]) || Argument_type[prod_tmp.right[j]] == TERMINAL) {
				seqence_first.push_back(prod_tmp.right[j]);
				break;
			}
			// 终结符
			else {
				for (int k = 0; k < First_set[prod_tmp.right[j]].size(); ++k) {
					if (!is_string_in_vector(seqence_first, First_set[prod_tmp.right[j]][k])) {
						seqence_first.push_back(First_set[prod_tmp.right[j]][k]);
					}
				}
				// 当前变元first集不含epsilon
				if (!is_string_in_vector(First_set[prod_tmp.right[j]], this->Grammar_symbol_mp["epsilon"]))
					break;
			}
			// lf
			if (j == prod_tmp.right.size() - 1) {
				seqence_first.push_back(lookforward_tmp);
			}
		}
		// 特判
		if (seqence_first.size() == 0)
			seqence_first.push_back(lookforward_tmp);
		// 获取该变元的产生式索引
		std::vector<int> argument_production_idx = this->Argument_garmmar_mp[prod_tmp.right[prod_tmp.dot_pos]];
		for (int i = 0; i < argument_production_idx.size(); ++i) {
			production prod_new = this->Production_raw[argument_production_idx[i]];
			item im_new = item(prod_new, 0);
			// qwait.push
			for (int j = 0; j < seqence_first.size(); ++j) {
				im_new.lookforward = seqence_first[j];
				qwait.push(im_new);
			}
		}// 遍历产生式
	}// end of while(q)
	return ret_ig;
}

// 打印"Start"的闭包（调试）
void Parser::show_closure() {
	production toprint = Production_raw[0];
	item im;
	im.prod = toprint;
	im.prod.dot_pos = 0;
	im.lookforward = "#";
	// itemGroup ret_ig = get_single_closer(im);
	itemGroup ig_tmp;
	ig_tmp.id = -1;
	ig_tmp.item_group.insert(im);
	itemGroup ret_ig = get_closer(ig_tmp);
	for (auto it = ret_ig.item_group.begin(); it != ret_ig.item_group.end(); ++it) {
		// (*it) - item
		// item - production + lookforward
		// production - left + dot + right
		std::cout << (*it).prod.left << "->";
		for (int i = 0; i < (*it).prod.right.size(); ++i) {
			if (i == (*it).prod.dot_pos) std::cout << "·";
			std::cout << (*it).prod.right[i];
		}
		if ((*it).prod.dot_pos == (*it).prod.right.size())
			std::cout << "·";
		std::cout << "," << (*it).lookforward << "\n";
	}
}

// 获取 get_closer 的输入项目集（根据·后不同的字符会产生多个itemGroup）
// 输入：I_i
std::vector<itemGroup> Parser::get_GOTO_input(const itemGroup& in_ig)
{
	std::vector<itemGroup> ret_igv;
	std::map<std::string, int> Item_idx;
	// 根据点后的字符分类获取转移索引
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		item im = (*it);
		// 点在末尾（添加一张表记录[I_i,s,j]，创建LR1分析表时有用）
		// Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,lf]=j
		if (im.prod.dot_pos >= im.prod.right.size()) {
			// for ACTION case3
			// A!=S'
			if (im.prod.left == "Start")
				continue;
			// lf.type == TERMINAL
			if (!Argument_type.count(im.lookforward) || Argument_type[im.lookforward] == TERMINAL) {
				this->Prod_acc_mp[im.prod.id][im.lookforward].push_back(in_ig.id);
			}
			continue;
		}
		// 获取点后字符
		std::string nxt = im.prod.right[im.prod.dot_pos];
		// 如果是空产生式，忽略
		if (nxt == this->Grammar_symbol_mp["epsilon"])
			continue;
		// 全局唯一挪"点位置"的语句
		im.prod.dot_pos++;
		// 已有，转移方向1
		if (Item_idx.count(nxt)) {
			int idx = Item_idx[nxt];
			ret_igv[idx].item_group.insert(im);
		}
		// 还未有，转移方向2
		else {
			Item_idx[nxt] = (int)ret_igv.size();
			itemGroup ig_new;
			ig_new.id = in_ig.id;
			ig_new.item_group.insert(im);
			ret_igv.push_back(ig_new);
		}
	}
	// debug
	//show_GOTO_input(ret_igv);
	// debug
	return ret_igv;
}

// 打印 get_closer 的输入项目集
void Parser::show_GOTO_input(std::vector<itemGroup> ig_v)
{
	for (int i = 0; i < ig_v.size(); ++i) {
		std::cout << i << std::endl;
		for (auto it = ig_v[i].item_group.begin(); it != ig_v[i].item_group.end(); ++it) {
			std::cout << (*it).prod.left << "->";
			for (int i = 0; i < (*it).prod.right.size(); ++i) {
				if (i == (*it).prod.dot_pos) std::cout << "·";
				std::cout << (*it).prod.right[i];
			}
			if ((*it).prod.dot_pos == (*it).prod.right.size())
				std::cout << "·";
			std::cout << "," << (*it).lookforward << "\n";
		}
	}
}

// 获取项目集的 closer
itemGroup Parser::get_closer(const itemGroup& in_ig)
{
	itemGroup ret_ig;
	ret_ig.id = in_ig.id;
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		itemGroup ig_tmp = get_single_closer((*it));
		ret_ig.itemGroupUnion(ig_tmp);
	}
	return ret_ig;
}

// 获取所有的项目集 - 项目集族
void Parser::get_item_group()
{
	// I_i --s--> I_j	vector<map<string,int>> ig_shift_mp; // 状态 I_i 点后为 s 转移到状态 I_j
	// Start
	production p_start = Production_raw[0];
	item im_start;
	im_start.prod = p_start;
	im_start.prod.dot_pos = 0;
	im_start.lookforward = "#";
	itemGroup ig_start;
	ig_start.id = -1;
	ig_start.item_group.insert(im_start);
	// qwait
	std::queue<itemGroup> qwait;
	qwait.push(ig_start);
	// while until 项目集族不再扩大
	while (qwait.size()) {
		itemGroup ig_tmp = qwait.front();
		qwait.pop();
		// 获取闭包I_i
		itemGroup ret_ig = get_closer(ig_tmp);
		// 1.如果是空包
		if (ret_ig.item_group.size() == 0)
			continue;
		// 2.如果该闭包已经存在
		if (this->Item_group_set.count(ret_ig)) {
			auto ig_exist = this->Item_group_set.find(ret_ig);
			auto it = ig_tmp.item_group.begin();
			// for ACTION case1/2
			Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = (*ig_exist).id;
			continue;
		}
		// 3.如果该闭包不存在
		else {
			ret_ig.id = (this->Item_group_num++);
			this->Item_group_set.insert(ret_ig);
			this->Item_group_shift_mp.resize(this->Item_group_num);
			// for ACTION case1/2
			if (ig_tmp.id >= 0) {
				auto it = ig_tmp.item_group.begin();
				Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = ret_ig.id;
			}
		}
		// 获取新闭包的vector<转移输入集>
		std::vector<itemGroup> goto_input = get_GOTO_input(ret_ig);
		// 如果产生式的点均已到结尾
		if (goto_input.size() == 0)
			continue;
		// 尚有待转移的
		for (int i = 0; i < goto_input.size(); ++i) {
			qwait.push(goto_input[i]);
		}
	}
}

// 打印项目集族
void Parser::show_item_group()
{
	std::ofstream outfile;
	outfile.open(PS_ItemGroup_Path, std::ios::out | std::ios::binary);
	for (auto it1 = this->Item_group_set.begin(); it1 != this->Item_group_set.end(); ++it1) {
		//if ((*it1).id != 80 && (*it1).id != 44)continue;
		outfile << "I_" << (*it1).id << "：\n";
		itemGroup ig_tmp = (*it1);
		for (auto it2 = ig_tmp.item_group.begin(); it2 != ig_tmp.item_group.end(); ++it2) {
			outfile << (*it2).prod.left << "->";
			for (int i = 0; i < (*it2).prod.right.size(); ++i) {
				if (i == (*it2).prod.dot_pos) outfile << "·";
				outfile << (*it2).prod.right[i];
			}
			if ((*it2).prod.dot_pos == (*it2).prod.right.size())
				outfile << "·";
			outfile << ",lf=" << (*it2).lookforward << "\n";
		}
	}
	outfile.close();
}

// 打印DFA状态转移表
void Parser::show_item_group_shift_mp()
{
	// std::vector<std::map<std::string, int>> Item_group_shift_mp;
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		std::cout << "State" << i << "：\n";
		if (this->Item_group_shift_mp[i].size() == 0) {
			std::cout << "Accepted State.\n";
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			std::cout << (*it).first << "--" << (*it).second << "\n";
		}
	}
}

// 获取LR1分析表
void Parser::get_LR1_table()
{
	// 通过 Item_group_shift_mp 获取 LR1 分析表
	this->LR1_table.resize(this->Item_group_shift_mp.size());
	// 分4种情况
	// 1. [A->..·a..,b] && goto(I_i,a)=I_j -> ACTION[i,a]=sj
	// 2. [A->..·B..,b] && goto(I_i,B)=I_j -> GOTO[i,B]=gj
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		if (this->Item_group_shift_mp[i].size() == 0) {
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			if(!Argument_type.count((*it).first) || Argument_type[(*it).first] == TERMINAL)
				this->LR1_table[i][(*it).first] = { 's',(*it).second };
			else
				this->LR1_table[i][(*it).first] = { 'g',(*it).second };
		}
	}
	// 3, [A->..·,a] && A!="Start" - ACTION[i,a]=rj
	// 格式：Prod_acc_mp[i]map<lf, vector<j>> -> ACTION[j, lf] = i
	for (int i = 0; i < Prod_acc_mp.size(); ++i) {
		// 相同的产生式共享展望符
		std::set<std::string> lf_set;
		for (auto it = Prod_acc_mp[i].begin(); it != Prod_acc_mp[i].end(); ++it) {
			lf_set.insert((*it).first);
		}
		for (auto it = Prod_acc_mp[i].begin(); it != Prod_acc_mp[i].end(); ++it) {
			for (auto it1 = lf_set.begin(); it1 != lf_set.end(); ++it1) {
				for (int j = 0; j < (*it).second.size(); ++j) {
					this->LR1_table[(*it).second[j]][(*it1)] = { 'r',i };
				}
			}
		}
	}
	// 4. [Start->mp[Program]·,#] -> ACTION[i,#]=ax
	this->LR1_table[1]["#"] = { 'a',0 };
}

// 打印LR1分析表
void Parser::show_LR1_table()
{
	// std::cout << this->LR1_table.size();
	std::ofstream outfile;
	outfile.open(PS_LR1table_Path, std::ios::out | std::ios::binary);
	for (int i = 0; i < this->LR1_table.size(); ++i) {
		outfile << "State" << i << ":\n";
		for (auto it = this->LR1_table[i].begin(); it != this->LR1_table[i].end(); ++it) {
			outfile << (*it).first <<" - " << (*it).second.first << (*it).second.second << "\n";
		}
	}
	outfile.close();
}

// 使用LR1分析表进行分析
bool Parser::Analyse(std::vector<token> tk)
{
	// 栈底状态
	int cur_state = 0;
	// 语法分析的状态
	int als_state = PARSER_GOING;
	// 状态栈 vector<int> StateStack;
	std::vector<int> StateStack;
	// 符号栈 vector<string> SymbolStack;
	std::vector<std::string> SymbolStack;
	// 步骤数
	int StepCount = 0;
	// 输入串指针（索引token[] tk）
	int p_tk = 0;
	// 初始化
	StateStack.push_back(cur_state);
	SymbolStack.push_back("#");
	// 输出控制
	std::ofstream outfile;
	outfile.open(PS_Rignt_Path, std::ios::out | std::ios::binary);
	// 分析
	while (als_state == 0 && p_tk < tk.size()) {
		StepCount++;
		// file - 输出栈的形态
		outfile << "Step" << StepCount << ":\n";
		outfile << "StateStack:";
		for (int i = 0; i < StateStack.size(); ++i) {
			outfile << StateStack[i] << " ";
		}
		outfile << "\n";
		outfile << "SymbolStack:";
		for (int i = 0; i < SymbolStack.size(); ++i) {
			outfile << SymbolStack[i] << " ";
		}
		outfile << "\n";
		// file - 输出栈的形态
		// 状态栈顶
		cur_state = StateStack[StateStack.size() - 1];
		token tk_tmp = tk[p_tk];
		// 输入串首元素处理
		std::string tk_tmp_str;
		if (tk_tmp.token_type == "identifier")
			tk_tmp_str = this->Grammar_symbol_mp["ID"];
		else if (tk_tmp.token_type == "digit")
			tk_tmp_str = this->Grammar_symbol_mp["num"];
		else
			tk_tmp_str = tk_tmp.token_type;
		// 判断err
		if (!this->LR1_table[cur_state].count(tk_tmp_str)) {
			// err
			als_state = ERROR_SHIFT;
			break;
		}
		// 移进/规约/goto/acc
		char action = this->LR1_table[cur_state][tk_tmp_str].first;
		int nxt_state = this->LR1_table[cur_state][tk_tmp_str].second;
		// file - action  eg.s1/r2...
		outfile << "action:" << action << nxt_state << "\n";
		// file
		// 1.移进
		if (action == 's') {
			SymbolStack.push_back(tk_tmp_str);
			StateStack.push_back(nxt_state);
			p_tk++;
		}
		// 2.规约
		else if (action == 'r') {
			production prod_to_use = Production_raw[nxt_state];
			for (int i = (int)prod_to_use.right.size() - 1; i >= 0; --i) {
				if (SymbolStack.size() > 1 && SymbolStack.back() == prod_to_use.right[i]) {
					SymbolStack.pop_back();
					StateStack.pop_back();
				}
				else {
					// err
					als_state = ERROR_REDUCE;
					break;
				}
			}
			// err
			if (als_state == ERROR_REDUCE)
				continue;
			SymbolStack.push_back(prod_to_use.left);
			// 规约完要多做一步goto
			cur_state = StateStack[StateStack.size() - 1];
			tk_tmp_str = SymbolStack[SymbolStack.size() - 1];
			if (!this->LR1_table[cur_state].count(tk_tmp_str)){
				// err
				als_state = ERROR_GOTO;
			}
			else {
				StateStack.push_back(this->LR1_table[cur_state][tk_tmp_str].second);
			}
		}
		// 3.goto 应该不会执行到，因为goto紧接在规约后做了
		else if (action == 'g') {
			StateStack.push_back(nxt_state);
		}
		// 4.acc
		else {
			als_state = PARSER_RIGHT;
			break;
		}
	}// while
	if (als_state == PARSER_RIGHT) {
		std::cout << "语法分析成功\n";
		outfile << "Parsing Success!\n";
		outfile.close();
		return true;
	}
	std::cout << "语法分析出错\n";
	std::cout << "错误代码:" << als_state << "\n";
	std::cout << "错误位置:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	// file
	outfile.open(PS_Wrong_Path, std::ios::out | std::ios::binary);
	outfile << "Parsing Failed!\n";
	outfile << "Return code:" << als_state << "\n";
	outfile << "Wrong place:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	return false;
}

/* END - class Parser - 函数实现 */