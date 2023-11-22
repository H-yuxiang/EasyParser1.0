#include "tool.h"
#include "parser.h"

/* BEGIN - class Parser - ����ʵ�� */
// ����
Parser::Parser()
{
	this->Item_group_num = 0;
}

// ����
Parser::~Parser()
{
}

// ��Ԫӳ���
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

// ��ʼ�ķ�
void Parser::load_production_raw()
{
	// ��չ
	Production_raw.push_back(production(0, ARGUMENT, -1, (std::string)("Start"), std::vector<std::string>{Grammar_symbol_mp[Grammar_rule[0][0]]}));
	this->Argument_garmmar_mp[(std::string)("Start")].push_back((int)Production_raw.size() - 1);
	// ����
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

// ��ӡ��ʼ�ķ������ԣ�
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

// ��ȡ������Ԫ��FIRST��
std::vector<std::string> Parser::get_single_first(std::string argument)
{
	std::vector<std::string> ret_first_set;
	// �������� argument Ϊ left �Ĳ���ʽ
	for (int idx = 0; idx < Argument_garmmar_mp[argument].size(); ++idx) {
		int i = Argument_garmmar_mp[argument][idx]; // �ñ�Ԫ�����в���ʽ����
		// Production_raw[i].right[0] �������ս��
		if (!Argument_type.count(Production_raw[i].right[0]) || Production_raw[i].right[0] == Grammar_symbol_mp["ID"] 
			|| Production_raw[i].right[0] == Grammar_symbol_mp["num"] || Production_raw[i].right[0] == Grammar_symbol_mp["epsilon"]) {
			if(!is_string_in_vector(ret_first_set, Production_raw[i].right[0]))
				ret_first_set.push_back(Production_raw[i].right[0]);
		}
		// Production_raw[i].right[0] �����Ǳ�Ԫ
		else {
			// ��󿴣�ֱ����1.ĳ���ս������2.ĳ����Ԫ��FIRST����epsilon��
			for (int j = 0; j < Production_raw[i].right.size(); ++j) {
				// ������ս��
				if (!Argument_type.count(Production_raw[i].right[j]) || Production_raw[i].right[j] == Grammar_symbol_mp["ID"]
					|| Production_raw[i].right[j] == Grammar_symbol_mp["num"] || Production_raw[i].right[j] == Grammar_symbol_mp["epsilon"]) {
					if (!is_string_in_vector(ret_first_set, Production_raw[i].right[j]))
						ret_first_set.push_back(Production_raw[i].right[j]);
					break; //1.
				}
				// ����Ǳ�Ԫ
				else {
					std::vector<std::string> tmp_first_set;
					// �ñ�Ԫ�Ѿ����FIRST
					if (First_set.count(Production_raw[i].right[j]))
						tmp_first_set = First_set[Production_raw[i].right[j]];
					else {
						// �ݹ�
						tmp_first_set = get_single_first(Production_raw[i].right[j]);
						First_set[Production_raw[i].right[j]] = tmp_first_set;
					}
					// ����FIRST
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

// ��ȡ���б�Ԫ��FIRST��
void Parser::get_first()
{
	for (int i = 0; i < this->Production_raw.size(); ++i) {
		// ����ʽ raw �� left ��Ȼ�Ǳ�Ԫ
		if (First_set.count(Production_raw[i].left))
			continue;// �Ѿ����
		First_set[Production_raw[i].left] = get_single_first(Production_raw[i].left);
	}
}

// ��ӡ���б�Ԫ��FIRST��
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

// ��ȡһ����Ŀ�ıհ��������� item �� dot��λ�ö����� right �Ŀ�ͷ��
itemGroup Parser::get_single_closer(item im)
{
	itemGroup ret_ig;
	// ���λ����ĩβ
	if (im.prod.dot_pos >= im.prod.right.size()) {
		ret_ig.item_group.insert(im);
		return ret_ig;
	}
	std::queue<item> qwait; // �ȴ�����
	qwait.push(im);
	while (qwait.size()) {
		item im_tmp = qwait.front();
		// ��ӵ� ret_ig
		ret_ig.item_group.insert(im_tmp);
		qwait.pop();
		production prod_tmp = im_tmp.prod;
		std::string lookforward_tmp = im_tmp.lookforward;
		// ����1�����ڲ���ʽ��ĩβ������������
		if (prod_tmp.dot_pos >= prod_tmp.right.size()) {
			continue;
		}
		// ����2�����Ϊ�ս��������������
		if (!Argument_type.count(prod_tmp.right[prod_tmp.dot_pos]) || Argument_type[prod_tmp.right[prod_tmp.dot_pos]] == TERMINAL) {
			continue;
		}
		// ����3�����Ϊ��Ԫ
		// ��Ҫ����ƺ��"ʣ������first��"
		std::vector<std::string> seqence_first;
		// һ��
		for (int j = prod_tmp.dot_pos + 1; j < prod_tmp.right.size(); ++j) {
			// ���ս��
			if (!Argument_type.count(prod_tmp.right[j]) || Argument_type[prod_tmp.right[j]] == TERMINAL) {
				seqence_first.push_back(prod_tmp.right[j]);
				break;
			}
			// �ս��
			else {
				for (int k = 0; k < First_set[prod_tmp.right[j]].size(); ++k) {
					if (!is_string_in_vector(seqence_first, First_set[prod_tmp.right[j]][k])) {
						seqence_first.push_back(First_set[prod_tmp.right[j]][k]);
					}
				}
				// ��ǰ��Ԫfirst������epsilon
				if (!is_string_in_vector(First_set[prod_tmp.right[j]], this->Grammar_symbol_mp["epsilon"]))
					break;
			}
			// lf
			if (j == prod_tmp.right.size() - 1) {
				seqence_first.push_back(lookforward_tmp);
			}
		}
		// ����
		if (seqence_first.size() == 0)
			seqence_first.push_back(lookforward_tmp);
		// ��ȡ�ñ�Ԫ�Ĳ���ʽ����
		std::vector<int> argument_production_idx = this->Argument_garmmar_mp[prod_tmp.right[prod_tmp.dot_pos]];
		for (int i = 0; i < argument_production_idx.size(); ++i) {
			production prod_new = this->Production_raw[argument_production_idx[i]];
			item im_new = item(prod_new, 0);
			// qwait.push
			for (int j = 0; j < seqence_first.size(); ++j) {
				im_new.lookforward = seqence_first[j];
				qwait.push(im_new);
			}
		}// ��������ʽ
	}// end of while(q)
	return ret_ig;
}

// ��ӡ"Start"�ıհ������ԣ�
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
			if (i == (*it).prod.dot_pos) std::cout << "��";
			std::cout << (*it).prod.right[i];
		}
		if ((*it).prod.dot_pos == (*it).prod.right.size())
			std::cout << "��";
		std::cout << "," << (*it).lookforward << "\n";
	}
}

// ��ȡ get_closer ��������Ŀ�������ݡ���ͬ���ַ���������itemGroup��
// ���룺I_i
std::vector<itemGroup> Parser::get_GOTO_input(const itemGroup& in_ig)
{
	std::vector<itemGroup> ret_igv;
	std::map<std::string, int> Item_idx;
	// ���ݵ����ַ������ȡת������
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		item im = (*it);
		// ����ĩβ�����һ�ű��¼[I_i,s,j]������LR1������ʱ���ã�
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
		// ��ȡ����ַ�
		std::string nxt = im.prod.right[im.prod.dot_pos];
		// ����ǿղ���ʽ������
		if (nxt == this->Grammar_symbol_mp["epsilon"])
			continue;
		// ȫ��ΨһŲ"��λ��"�����
		im.prod.dot_pos++;
		// ���У�ת�Ʒ���1
		if (Item_idx.count(nxt)) {
			int idx = Item_idx[nxt];
			ret_igv[idx].item_group.insert(im);
		}
		// ��δ�У�ת�Ʒ���2
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

// ��ӡ get_closer ��������Ŀ��
void Parser::show_GOTO_input(std::vector<itemGroup> ig_v)
{
	for (int i = 0; i < ig_v.size(); ++i) {
		std::cout << i << std::endl;
		for (auto it = ig_v[i].item_group.begin(); it != ig_v[i].item_group.end(); ++it) {
			std::cout << (*it).prod.left << "->";
			for (int i = 0; i < (*it).prod.right.size(); ++i) {
				if (i == (*it).prod.dot_pos) std::cout << "��";
				std::cout << (*it).prod.right[i];
			}
			if ((*it).prod.dot_pos == (*it).prod.right.size())
				std::cout << "��";
			std::cout << "," << (*it).lookforward << "\n";
		}
	}
}

// ��ȡ��Ŀ���� closer
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

// ��ȡ���е���Ŀ�� - ��Ŀ����
void Parser::get_item_group()
{
	// I_i --s--> I_j	vector<map<string,int>> ig_shift_mp; // ״̬ I_i ���Ϊ s ת�Ƶ�״̬ I_j
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
	// while until ��Ŀ���岻������
	while (qwait.size()) {
		itemGroup ig_tmp = qwait.front();
		qwait.pop();
		// ��ȡ�հ�I_i
		itemGroup ret_ig = get_closer(ig_tmp);
		// 1.����ǿհ�
		if (ret_ig.item_group.size() == 0)
			continue;
		// 2.����ñհ��Ѿ�����
		if (this->Item_group_set.count(ret_ig)) {
			auto ig_exist = this->Item_group_set.find(ret_ig);
			auto it = ig_tmp.item_group.begin();
			// for ACTION case1/2
			Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = (*ig_exist).id;
			continue;
		}
		// 3.����ñհ�������
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
		// ��ȡ�±հ���vector<ת�����뼯>
		std::vector<itemGroup> goto_input = get_GOTO_input(ret_ig);
		// �������ʽ�ĵ���ѵ���β
		if (goto_input.size() == 0)
			continue;
		// ���д�ת�Ƶ�
		for (int i = 0; i < goto_input.size(); ++i) {
			qwait.push(goto_input[i]);
		}
	}
}

// ��ӡ��Ŀ����
void Parser::show_item_group()
{
	std::ofstream outfile;
	outfile.open(PS_ItemGroup_Path, std::ios::out | std::ios::binary);
	for (auto it1 = this->Item_group_set.begin(); it1 != this->Item_group_set.end(); ++it1) {
		//if ((*it1).id != 80 && (*it1).id != 44)continue;
		outfile << "I_" << (*it1).id << "��\n";
		itemGroup ig_tmp = (*it1);
		for (auto it2 = ig_tmp.item_group.begin(); it2 != ig_tmp.item_group.end(); ++it2) {
			outfile << (*it2).prod.left << "->";
			for (int i = 0; i < (*it2).prod.right.size(); ++i) {
				if (i == (*it2).prod.dot_pos) outfile << "��";
				outfile << (*it2).prod.right[i];
			}
			if ((*it2).prod.dot_pos == (*it2).prod.right.size())
				outfile << "��";
			outfile << ",lf=" << (*it2).lookforward << "\n";
		}
	}
	outfile.close();
}

// ��ӡDFA״̬ת�Ʊ�
void Parser::show_item_group_shift_mp()
{
	// std::vector<std::map<std::string, int>> Item_group_shift_mp;
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		std::cout << "State" << i << "��\n";
		if (this->Item_group_shift_mp[i].size() == 0) {
			std::cout << "Accepted State.\n";
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			std::cout << (*it).first << "--" << (*it).second << "\n";
		}
	}
}

// ��ȡLR1������
void Parser::get_LR1_table()
{
	// ͨ�� Item_group_shift_mp ��ȡ LR1 ������
	this->LR1_table.resize(this->Item_group_shift_mp.size());
	// ��4�����
	// 1. [A->..��a..,b] && goto(I_i,a)=I_j -> ACTION[i,a]=sj
	// 2. [A->..��B..,b] && goto(I_i,B)=I_j -> GOTO[i,B]=gj
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
	// 3, [A->..��,a] && A!="Start" - ACTION[i,a]=rj
	// ��ʽ��Prod_acc_mp[i]map<lf, vector<j>> -> ACTION[j, lf] = i
	for (int i = 0; i < Prod_acc_mp.size(); ++i) {
		// ��ͬ�Ĳ���ʽ����չ����
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
	// 4. [Start->mp[Program]��,#] -> ACTION[i,#]=ax
	this->LR1_table[1]["#"] = { 'a',0 };
}

// ��ӡLR1������
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

// ʹ��LR1��������з���
bool Parser::Analyse(std::vector<token> tk)
{
	// ջ��״̬
	int cur_state = 0;
	// �﷨������״̬
	int als_state = PARSER_GOING;
	// ״̬ջ vector<int> StateStack;
	std::vector<int> StateStack;
	// ����ջ vector<string> SymbolStack;
	std::vector<std::string> SymbolStack;
	// ������
	int StepCount = 0;
	// ���봮ָ�루����token[] tk��
	int p_tk = 0;
	// ��ʼ��
	StateStack.push_back(cur_state);
	SymbolStack.push_back("#");
	// �������
	std::ofstream outfile;
	outfile.open(PS_Rignt_Path, std::ios::out | std::ios::binary);
	// ����
	while (als_state == 0 && p_tk < tk.size()) {
		StepCount++;
		// file - ���ջ����̬
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
		// file - ���ջ����̬
		// ״̬ջ��
		cur_state = StateStack[StateStack.size() - 1];
		token tk_tmp = tk[p_tk];
		// ���봮��Ԫ�ش���
		std::string tk_tmp_str;
		if (tk_tmp.token_type == "identifier")
			tk_tmp_str = this->Grammar_symbol_mp["ID"];
		else if (tk_tmp.token_type == "digit")
			tk_tmp_str = this->Grammar_symbol_mp["num"];
		else
			tk_tmp_str = tk_tmp.token_type;
		// �ж�err
		if (!this->LR1_table[cur_state].count(tk_tmp_str)) {
			// err
			als_state = ERROR_SHIFT;
			break;
		}
		// �ƽ�/��Լ/goto/acc
		char action = this->LR1_table[cur_state][tk_tmp_str].first;
		int nxt_state = this->LR1_table[cur_state][tk_tmp_str].second;
		// file - action  eg.s1/r2...
		outfile << "action:" << action << nxt_state << "\n";
		// file
		// 1.�ƽ�
		if (action == 's') {
			SymbolStack.push_back(tk_tmp_str);
			StateStack.push_back(nxt_state);
			p_tk++;
		}
		// 2.��Լ
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
			// ��Լ��Ҫ����һ��goto
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
		// 3.goto Ӧ�ò���ִ�е�����Ϊgoto�����ڹ�Լ������
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
		std::cout << "�﷨�����ɹ�\n";
		outfile << "Parsing Success!\n";
		outfile.close();
		return true;
	}
	std::cout << "�﷨��������\n";
	std::cout << "�������:" << als_state << "\n";
	std::cout << "����λ��:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	// file
	outfile.open(PS_Wrong_Path, std::ios::out | std::ios::binary);
	outfile << "Parsing Failed!\n";
	outfile << "Return code:" << als_state << "\n";
	outfile << "Wrong place:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	return false;
}

/* END - class Parser - ����ʵ�� */