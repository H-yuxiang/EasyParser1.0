#include "common.h"

//����ʶ��token��DFA�Ļ���
class DFA
{
public:
    explicit DFA();
    //�Ƿ񱻸�DFA���ܣ����뱻��������
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) = 0;
};

//ʶ��ؼ��ֵ�DFA
class KeyWordsDFA :public DFA
{
private:
    std::map<std::string, int>KeyWordsmap;
public:
    KeyWordsDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//ʶ���ʶ����DFA
class IdentifierDFA :public DFA
{
public:
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//ʶ����ֵ��DFA
class DigitDFA :public DFA
{
private:
    //������ܵ���ֵ��״̬������/С����ǰ��/��ѧ������ǰ��/����/�յ�
    int state;
    int SInteger = 0;
    int SDecimalBefore = 1;
    int SDecimalAfter = 2;
    int SSCIBefore = 3;
    int SSCIAfter = 4;
    int SERROR = 5;
    int STerminal = 6;
public:
    DigitDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//ʶ������DFA
class DelimiterDFA :public DFA
{
public:
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//ʶ�������DFA
class OperatorDFA :public DFA
{
private:
    std::map<std::string, int> Operatormap;
public:
    OperatorDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//���дʷ���������
class Lexical
{
private:
    DFA* myDFA[5];
public:
    Lexical();
    ~Lexical();
    std::pair<int, std::vector<token>> Analyze(std::string& inpput);
};