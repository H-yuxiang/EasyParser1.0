#include "tool.h"
#include "Lexical.h"

/************************************************
��𣺸�DFA����
*************************************************/
DFA::DFA()
{
}

/************************************************
���ʶ��ؼ��ֵ�DFAʵ��
	����keywords��������std::mapping��ϵ
	�ж��Ƿ񱻹ؼ���DFA����
*************************************************/
KeyWordsDFA::KeyWordsDFA()
{
	for (int i = 0; i < KeyWord_NUM; i++) {
		KeyWordsmap[KeyWords[i]] = i;
	}
}
int KeyWordsDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;//��ǰ�����λ��
	int cur_col = col;//��ǰ������
	std::string nowWord = "";//��ǰʶ��ĵ���

	while (true) {
		if (isLetter(str[pos])) {
			nowWord += str[pos++];
			cur_col++;
		}
		else
			break;
	}

	auto it = KeyWordsmap.find(nowWord);
	if (it != KeyWordsmap.end()) {
		//����ǹؼ��ֵĻ�,���عؼ����ڹؼ��������е�����
		end = pos - 1;
		col = cur_col;
		return it->second;
	}

	return WRONG;
}

/*************************************************
���ʶ���ʶ����DFAʵ��
	�ж��Ƿ񱻱�ʶ��DFA����
	����ʶ������ı�ʶ��token�����ʶ������
*************************************************/
int IdentifierDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;//��ǰ�����λ��
	int cur_col = col;//��ǰ������
	std::string nowWord = "";//��ǰʶ��ĵ���
	while (true) {
		if (isLetter(str[pos]) || isDigit(str[pos])) {
			nowWord += str[pos++];
			cur_col++;
		}
		else
			break;
	}
	if (nowWord == "")
		return WRONG;
	end = pos - 1;
	col = cur_col;
	Identifier.insert(nowWord);//�����ʶ��
	return RIGHT;//��ʶ��ɹ���������ȷ
}

/*************************************************
���ʶ����ֵ��DFAʵ��
	���캯������״̬��ʼ��Ϊ��������
	�ж��Ƿ���ֵDFA����
*************************************************/
DigitDFA::DigitDFA()
{
	state = SInteger;
}
int DigitDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	if (state != SInteger)
		return WRONG;
	int pos = start;
	//�����ַ�ֱ�����������ֹ
	while (1) {
		if (pos > str.size())
			return WRONG;
		if (state == SInteger) {
			if (str[pos] >= '0' && str[pos] <= '9');
			else if (str[pos] == '.') state = SDecimalBefore;//С��֮ǰ ����235.
			else if (str[pos] == 'e')state = SSCIBefore;//��ѧ������֮ǰ ����455e
			else {
				state = STerminal;//��ֵ��ȡ����
				break;
			}
		}
		else if (state == SDecimalBefore) {
			//��ʱ����235.
			if (str[pos] >= '0' && str[pos] <= '9') state = SDecimalAfter;//С��֮������235.5
			else if (str[pos] == 'e') state = SSCIBefore;//��ѧ������֮ǰ ����455.e  ��455.0e���
			else if (str[pos] == '.') {
				state = SERROR;
				break;
			}//����123..  ����
			else {
				state = STerminal;
				break;
			}
		}
		else if (state == SDecimalAfter) {
			if (str[pos] >= '0' && str[pos] <= '9');
			else if (str[pos] == 'e') state = SSCIBefore;//��ѧ������֮ǰ ����455.1e
			else if (str[pos] == '.') {
				state = SERROR;
				break;
			}//����123.1.  ����
			else {
				state = STerminal;
				break;
			}
		}
		else if (state == SSCIBefore) {
			if (str[pos] >= '0' && str[pos] <= '9') state = SSCIAfter;//��ѧ������֮�� ����123e1
			else {
				state = SERROR;
				break;
			}
		}
		else if (state == SSCIAfter) {
			if (str[pos] >= '0' && str[pos] <= '9');//��ѧ������֮�� ����123e12
			else if (str[pos] == 'e' || str[pos] == '.') {
				state = SERROR;
				break;
			}
			else {
				state = STerminal;
			}
		}
		pos++;
	}
	if (state == STerminal) {
		state = SInteger;
		end = pos - 1;
		col += pos - start;
		std::string tmp = str.substr(start, pos - start);
		//��ʶ�𵽵��ַ�������ֵ��
		Digit.insert(tmp);
		return RIGHT;
	}
	//SERROR
	else {
		state = SInteger;
		return WRONG;
	}
}

/*************************************************
���ʶ������DFAʵ��
	���Ҹ��ַ��Ƿ��ڽ�������У�
	�ж��Ƿ񱻽��DFA����
*************************************************/
int DelimiterDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;
	for (int i = 0; i < Delimiter_NUM; i++) {
		//�ҵ���
		if (str.substr(pos).find(Delimiter[i]) != -1) {
			start = end = pos;
			col++;
			return RIGHT;
		}
	}
	return WRONG;
}

/*************************************************
���ʶ�������DFAʵ��
	����operator��������std::mapping��ϵ
	�ж��Ƿ��ܺ������������ƥ����
	ֻҪƥ�����ˣ��ͷ���
*************************************************/
OperatorDFA::OperatorDFA()
{
	for (int i = 0; i < Operator_NUM; i++)
		Operatormap[Operator[i]] = i;
}
int OperatorDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;
	char op = str[pos];
	if (op == '+') {
		char opp = str[pos + 1];
		if (opp == '+') end = pos + 1;// ++
		else if (opp == '=') end = pos + 1;// +=
		else end = pos;// +
	}
	else if (op == '-') {
		char opp = str[pos + 1];
		if (opp == '-') end = pos + 1;// --
		else if (opp == '=') end = pos + 1;// -=
		else if (opp == '>') end = pos + 1;// ->
		else end = pos;//  -
	}
	else if (op == '>') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// >=
		else if (opp == '>') {
			char oppp = str[pos + 2];
			if (oppp == '=') end = pos + 2;// >>=
			else end = pos + 1;// >>
		}
		else end = pos;// >
	}
	else if (op == '<') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// <=
		else if (opp == '<') {
			char oppp = str[pos + 2];
			if (oppp == '=') end = pos + 2;// <<=
			else end = pos + 1;// <<
		}
		else end = pos;// <
	}
	else if (op == '&') {
		char opp = str[pos + 1];
		if (opp == '&') end = pos + 1;//&&
		else if (opp == '=')end = pos + 1;//&=
		else end = pos;// &
	}
	else if (op == '|') {
		char opp = str[pos + 1];
		if (opp == '|')end = pos + 1;// ||
		else if (opp == '=')end = pos + 1;// |=
		else end = pos;// |
	}
	else if (op == '*' || op == '/' || op == '%' || op == '=' || op == '!' || op == '^') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// *= /= %= == != ^=
		else end = pos;//* / % = ! ^
	}
	else if (op == ',' || op == '?' || op == ':' || op == '.' || op == '~') {
		end = pos;
	}

	else
		return WRONG;

	col += end - start + 1;
	return RIGHT;
}

/*************************************************
��𣺴ʷ�����
*************************************************/
Lexical::Lexical()
{
	myDFA[ENTRANCE_KEYWORDS] = new KeyWordsDFA;
	myDFA[ENTRANCE_IDENTIFIER] = new IdentifierDFA;
	myDFA[ENTRANCE_OPERATOR] = new OperatorDFA;
	myDFA[ENTRANCE_DELIMITER] = new DelimiterDFA;
	myDFA[ENTRANCE_DIGIT] = new DigitDFA;
}
Lexical::~Lexical()
{
	for (int i = 0; i < 5; i++) {
		if (myDFA[i] != nullptr) {
			delete myDFA[i];
			myDFA[i] = nullptr;
		}
	}
}
std::pair<int, std::vector<token>> Lexical::Analyze(std::string& input) {
	// ��������
	std::string output = "";
	int row = 0, col = 0;
	int SIGN = LEXICAL_RIGHT;
	int start = 0, end = -1;
	int cur_row = 1, cur_col = 0;
	int last_row = cur_row, last_col = cur_col;
	std::stringstream final_str;

	// token��Ϣ
	std::vector<token> tk_info;

	// Ԥ����
	int pre_res = Preprocess(input, row, col, output);
	if (pre_res != RIGHT) {
		std::ofstream outfile(LC_Wrong_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "��ʧ��";
			return { -1 ,tk_info };
		}
		outfile << row << ' ' << col << "preprocess fail!\n" ;
		outfile.close();
		return { ERROR_Pre, tk_info };
	}
	
	// ����
	while (1) {
		start = end + 1;
		if (start >= output.size())
			break;
		//Ԥ�������outputʱ�������¿ո�ͻ��У��ڴ�����һ��tokenǰ����ȥ��
		int pos = start;
		while (output[pos] == ' ' || output[pos] == '\r' || output[pos] == '\n') {
			if (output[pos] == ' ' || output[pos] == '\r') {
				pos++;
				cur_col++;
			}
			else {
				pos++;
				cur_col = 0;
				cur_row++;
			}
			if (pos >= output.size())
				break;
		}
		start = pos;
		end = pos;
		//ǰ��û���Կո���н�β��˵���﷨������ȷ
		if (start >= output.size()) {
			SIGN = LEXICAL_RIGHT;
			break;
		}

		//��ʽ�жϵ�ǰ��Ч�ַ���DFA���
		int Entrance_DFA = -1;
		char ch = output[start];
		if (isOperator(ch)) Entrance_DFA = ENTRANCE_OPERATOR;
		else if (isDelimiter(ch)) Entrance_DFA = ENTRANCE_DELIMITER;
		else if (isDigit(ch)) Entrance_DFA = ENTRANCE_DIGIT;
		//�ؼ��ֺͱ�ʶ����DFAʶ�������Ѿ�����
		if (Entrance_DFA == -1) {
			//һ��Ҫ���ж��ǲ��ǹؼ��֣���ȥ�ж��ǲ��Ǳ�ʶ��
			int result = myDFA[ENTRANCE_KEYWORDS]->isAccepted(output, start, end, cur_row, cur_col);
			if (result == WRONG) {
				//���ǹؼ���
				result = myDFA[ENTRANCE_IDENTIFIER]->isAccepted(output, start, end, cur_row, cur_col);
				if (result == RIGHT)//�Ǳ�ʶ��
					Entrance_DFA = ENTRANCE_IDENTIFIER;
			}
			else//�ǹؼ���
				Entrance_DFA = ENTRANCE_KEYWORDS;

			//����˵Entrance_DFA����Ϊ-1
			if (Entrance_DFA == -1) {
				//todo,����ʶ��
				SIGN = error_sign(-1);
			}
		}
		//�������������������ֵ�DFAʶ��
		else {
			int result = myDFA[Entrance_DFA]->isAccepted(output, start, end, cur_row, cur_col);
			if (result == WRONG) {
				//todo,����ʶ��
				SIGN = error_sign(Entrance_DFA);
			}
		}
		//�жϱ���ѭ���Ƿ񵽽���
		if (SIGN != LEXICAL_RIGHT)//��ʶ��token�Ĺ����г���
			break;
		//����tokenʶ��ɹ�
		else {
			std::string token_tmp = output.substr(start, end - start + 1);
			std::string token_type;
			create_token_line(Entrance_DFA, token_tmp, token_type);
			final_str << cur_row << ' ' << cur_col << ' ' << token_type << ' ' << token_tmp << "\n";
			tk_info.push_back(token(cur_row, cur_col, token_type, token_tmp));
			last_row = cur_row;
			last_col = cur_col;
		}
	}

	//�ı�ʶ�����
	//�ʷ�������ȷ��������ļ�
	if (SIGN == LEXICAL_RIGHT) {
		std::ofstream outfile(LC_Rignt_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "��ʧ��";
			return { -1,tk_info };
		}
		outfile << final_str.str();
		outfile << last_row << ' ' << 1 << ' ' << "#" << ' ' << "#" << "\n";
		tk_info.push_back(token(last_row, 1, "#", "#"));
		outfile.close();
	}
	//�ʷ���������
	else {
		std::ofstream outfile(LC_Wrong_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "��ʧ��";
			return { -1,tk_info };
		}
		std::string token = output.substr(start, end - start + 1);
		std::string token_type;
		create_sign_error_line(SIGN, token, token_type);
		outfile << last_row << ' ' << last_col + 1 << ' ' << token_type << ' ' << token << "\n";
		outfile.close();
	}
	return { SIGN,tk_info };
}
