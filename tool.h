#pragma once
#include "common.h"

// 工具函数
bool isLetter(char ch);
bool isDigit(char ch);
bool isOperator(char ch);
bool isDelimiter(char ch);
int error_sign(int entrance);
void create_token_line(int entrance_dfa, std::string token, std::string& token_type);
void create_sign_error_line(int sign, std::string token, std::string& token_type);

// 预处理
int Preprocess(std::string& input, int& row, int& col, std::string& output);

// 代码读取
std::string file_to_str(std::string path);

// 工具类
// class token - 记录词法分析处理后的token
class token
{
public:
	token();
	~token();

	token(int tx, int ty, std::string type, std::string value);

//private:
	int token_x, token_y;
	std::string token_type;
	std::string token_value;
};

// 工具类
// class production - 记录文法的产生式
class production
{
public:
	// function
	production();
	~production();

	production(int _id, int _type,int _dot_pos,std::string _left, std::vector<std::string> _right);

	// data
	int id;
	int type;
	int dot_pos;
	std::string left;
	std::vector<std::string> right;

	// operator=
	production& operator=(const production& p) {
		if (this != &p) {
			this->id = p.id;
			this->type = p.type;
			this->dot_pos = p.dot_pos;
			this->left = p.left;
			this->right = p.right;
		}
		return *this;
	}
	bool operator==(const production& p) {
		if (id != p.id)return false;
		if (type != p.type)return false;
		if (dot_pos != p.dot_pos)return false;
		if (left != p.left)return false;
		if (right != p.right)return false;
		return true;
	}
};

// 工具函数 - 在 vector<string> 中查看某一个 string s 是否出现过
bool is_string_in_vector(std::vector<std::string> string_set, std::string s);

// 工具类
// class item - 项目
class item {
public:
	// function
	item();
	~item();

	item(production p, int _dot_pos);

	//data
	// 产生式(+点的位置)
	production prod;
	// 展望符
	std::string lookforward;

	// operator
	bool operator<(const item& a) const
	{
		if (this->prod.id != a.prod.id)
			return this->prod.id < a.prod.id;
		if (this->prod.type != a.prod.type)
			return this->prod.type < a.prod.type;
		if (this->prod.dot_pos != a.prod.dot_pos)
			return this->prod.dot_pos < a.prod.dot_pos;
		if (this->prod.left != a.prod.left)
			return this->prod.left < a.prod.left;
		if (this->prod.right.size() != a.prod.right.size())
			return this->prod.right.size() < a.prod.right.size();
		for (int i = 0; i < this->prod.right.size(); ++i) {
			if (this->prod.right[i] != a.prod.right[i])
				return this->prod.right[i] < a.prod.right[i];
		}
		return lookforward < a.lookforward;
	}

	bool operator==(const item& a) const
	{
		if (this->prod.id != a.prod.id)
			return false;
		if (this->prod.type != a.prod.type)
			return false;
		if (this->prod.dot_pos != a.prod.dot_pos)
			return false;
		if (this->prod.left != a.prod.left)
			return false;
		if (this->prod.right.size() != a.prod.right.size())
			return false;
		for (int i = 0; i < this->prod.right.size(); ++i) {
			if (this->prod.right[i] != a.prod.right[i])
				return false;
		}
		return true;
	}
};

// 工具类
// class itemGroup - 项目集
class itemGroup
{
public:
	// function
	itemGroup();
	~itemGroup();

	itemGroup(int _id);

	// 合并项目集 
	// return : 合并前后是否相等
	bool itemGroupUnion(const itemGroup& otherIG);

	// data
	int id; // I_id
	std::set<item> item_group;

	// operator
	bool operator<(const itemGroup& a) const
	{
		// id不考虑
		if (this->item_group.size() != a.item_group.size()) {
			return this->item_group.size() < a.item_group.size();
		}
		for (auto it1 = this->item_group.begin(),it2= a.item_group.begin(); it1 != this->item_group.end(); ++it1,++it2) {
			if (!((item)(*it1) == (*it2)))
				return (*it1) < (*it2);
		}
		return false;
	}

	bool operator==(const itemGroup& a) const
	{
		if (this->item_group.size() != a.item_group.size()) {
			return false;
		}
		for (auto it1 = this->item_group.begin(), it2 = a.item_group.begin(); it1 != this->item_group.end(); ++it1, ++it2) {
			if (!((item)(*it1) == (*it2)))
				return false;
		}
		return true;
	}
};
