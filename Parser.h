#pragma once
#include "common.h"

/*
	�ࣺ�﷨���� - Parser
*/
class Parser
{
public:
	Parser();
	~Parser();

	/*
		����1�����ر�Ԫӳ���
		return:	map<string, string>
	*/
	void load_argument_def();

	/*
		����2�����س�ʼ�ķ�������չ��
		return:	vector<production>
	*/
	void load_production_raw();
	void show_production_raw();

	/*
		����3����ȡ������Ԫ��FIRST��
		return: map<string, vector<string>>
	*/
	std::vector<std::string> get_single_first(std::string argument);

	/*
		����4����ȡ���б�Ԫ��FIRST��
		return: map<string, vector<string>>
	*/
	void get_first();
	void show_first();

	/*
		����5����ȡһ����Ŀ�ıհ�
		return: ����Ŀ�ıհ����ɵ���Ŀ��
	*/
	itemGroup get_single_closer(item im);
	void show_closure();

	/*
		����6����ȡ get_closer ������ - get_item_group seris 1
		input: һ����Ŀ��
		return: һ��<�����ɱհ�����Ŀ>�ļ���
	*/
	std::vector<itemGroup> get_GOTO_input(const itemGroup& in_ig);
	void show_GOTO_input(std::vector<itemGroup> ig_v);

	/*
		����7����ȡ��Ŀ���� closer - get_item_group seris 2
		return: I_k - һ�����ڹ淶�����Ŀ��
	*/
	itemGroup get_closer(const itemGroup& in_ig);

	/*
		����8����ȡ��Ŀ���淶�壬����洢�� std::set<itemGroup> Item_group_set;
	*/
	void get_item_group();
	void show_item_group();
	void show_item_group_shift_mp();

	/*
		����9����ȡLR1����������洢�� LR1_table
	*/
	void get_LR1_table();
	void show_LR1_table();

	/*
		����10��ʹ��LR1��������з���
		return: �����Ƿ�ɹ�
	*/
	bool Analyse(std::vector<token> tk);

private:
	// 1.��Ԫӳ��������ƣ�һһ��Ӧ��
	std::map<std::string, std::string> Grammar_symbol_mp;
	// 2."��Ԫ-�ķ�"ӳ��� - �ñ�ԪΪ�󲿵����в���ʽ�������� load_production_raw ����
	//	  				   - �������� Grammar_symbol_mp[...] ӳ��������
	std::map<std::string, std::vector<int>> Argument_garmmar_mp;
	// 3.��Ԫ��������
	// Tips���ж� X �Ƿ����ս�� -> !Argument_type.count(X) || Argument_type[X] == TERMINAL
	std::unordered_map<std::string, int> Argument_type;
	// 4.��ʼ�ķ�������չ��
	std::vector<production> Production_raw;
	// 5.���б�Ԫ��FIRST��
	std::map<std::string, std::vector<std::string>> First_set;

	// ��Ŀ����
	// 1.��Ŀ������
	int Item_group_num;
	// 2.[I_i-(s)->I_j] - ״̬I_i���Ϊs��ת�Ƶ�״̬I_j - ������
	std::vector<std::map<std::string,int>> Item_group_shift_mp;
	// 3.��Ŀ����set
	std::set<itemGroup> Item_group_set;
	// 4.����ʽ��ԼACTION��������ݣ����ڲ���ʽ�Ҳ������ - ����(size)=����ʽ�ĸ���
	// ��¼�ĸ�ʽ��Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,a]=j
	std::vector<std::map<std::string, std::vector<int>>> Prod_acc_mp;

	// LR(1)������
	// ACTION��GOTOͨ����������
	// ����pair<s(�ƽ�)/r(��Լ)/a(accept)/g(goto), int(next state)>
	std::vector< std::map<std::string, std::pair<char, int>> > LR1_table;
};

