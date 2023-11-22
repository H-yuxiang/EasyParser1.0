#include "tool.h"
#include "Lexical.h"
#include "Parser.h"

int main() {

    std::string input = file_to_str(Input_Path);

    Lexical LC;
    std::pair<int, std::vector<token>> is_LC_right = LC.Analyze(input);
    if(is_LC_right.first > 0){
        std::cout << "词法分析成功\n";
    }
    else {
        std::cout << "词法分析失败\n";
        return 0;
    }

    Parser PS;
    PS.load_argument_def();
    PS.load_production_raw();
    PS.show_production_raw();
    PS.get_first();
    // PS.show_first();
    // PS.show_closure();
    PS.get_item_group();
    PS.show_item_group();
    // PS.show_item_group_shift_mp();
    PS.get_LR1_table();
    PS.show_LR1_table();
    PS.Analyse(is_LC_right.second);

    return 0;
}
