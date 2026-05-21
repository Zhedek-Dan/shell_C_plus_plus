#include "scanner.h"
#include <cctype>
#include <stdexcept>

const char* Scanner::TW[] = {                       
    "",
    "and", "begin", "bool", "do", "else", "end",
    "if", "false", "int", "real", "string", "not", "or",
    "program", "read", "then", "true", "var",
    "while", "continue", "write", "fin", nullptr};

const char* Scanner::TD[] = {
    "",
    ",", ";", ":", "=",
    "(", ")", "==", "<", ">", "+", "-", "*", "/", "<=", ">=", "!=",
    "{", "}", nullptr};

const char* Lex::lex_type_names[] = {
    "LEX_NULL",
    // служебные слова
    "LEX_AND",
    "LEX_BEGIN",
    "LEX_BOOL",
    "LEX_DO",
    "LEX_ELSE",
    "LEX_END",
    "LEX_IF",
    "LEX_FALSE",
    "LEX_INT",
    "LEX_REAL",
    "LEX_STRING",
    "LEX_NOT",
    "LEX_OR",
    "LEX_PROGRAM",
    "LEX_READ",
    "LEX_THEN",
    "LEX_TRUE",
    "LEX_VAR",
    "LEX_WHILE",
    "LEX_CONTINUE",
    "LEX_WRITE",
    "LEX_FIN",
    // разделители
    "LEX_SEMICOLON",
    "LEX_COMMA",
    "LEX_COLON",
    "LEX_ASSIGN",
    "LEX_LPAREN",
    "LEX_RPAREN",
    "LEX_EQ",
    "LEX_LT",
    "LEX_GT",
    "LEX_PLUS",
    "LEX_MINUS",
    "LEX_STAR",
    "LEX_SLASH",
    "LEX_LE",
    "LEX_GE",
    "LEX_NE",
    "LEX_LBRACE",
    "LEX_RBRACE",
    // константы и идентификатор
    "LEX_INT_val",
    "LEX_REAL_val",
    "LEX_STRING_val",
    "LEX_ID",
    // ПОЛИЗ
    "POLIZ_ADDR",
    "POLIZ_GO",
    "POLIZ_FGO",
    "POLIZ_READ",
    "POLIZ_WRITE",
    "LEX_UMINUS",
    // конец файла
    "LEX_EOF"
};

void Scanner::skip_spaces_and_comments() {
    while (true) {
        while (isspace(ch)) {
            get_next_ch();
        }

        if (ch == '/') {
            int next = fgetc(f); 
            if (next == '*') {  // точно комментарий          
                while (true) {
                    int try1 = fgetc(f);
                    if (try1 == EOF) 
                    	error("Незакрытый комментарий");
                    if (try1 == '*') {
                        try1 = fgetc(f);
                        if (try1 == '/') 
                        	break;
                    }
                }
                get_next_ch(); 
                continue; // вдруг пробелы после комментария
            } 
            else {
                ungetc(next, f); // знак деления, возвращаем символ после / обратно
                break;
            }
        } 
        else
            break;
    }
}

int Scanner::put_TID(const std::string& name) {
    int i = find_TID(name);
    if (i != -1) 
    	return i;
    TID.push_back(Ident(name));
    return int(TID.size()) - 1;
}

int Scanner::find_TID(const std::string& name) { // TID - динамический массив
    for (size_t i = 0; i < TID.size(); i++)
        if (name == TID[i].get_name()) 
        	return int(i); // i - беззнаковый
    return -1;
}

Lex Scanner::read_number() {
    std::string num;
    bool is_real = false;
    while (isdigit(ch)) {
        num += ch;
        get_next_ch();
    }
    if (ch == '.') {
        is_real = true;
        num += ch;
        get_next_ch();
        if (!isdigit(ch)) error("Ожидаются цифры после точки");
        while (isdigit(ch)) {
            num += ch;
            get_next_ch();
        }
    }
    current_lexeme = num;
    if (is_real) {
        double val = std::stod(num);
        return Lex(val);
    } else {
        int val = std::stoi(num);
        return Lex(val);
    }
}

Lex Scanner::read_string() { 
    std::string str;
    current_lexeme = "\"";
    get_next_ch();
    while (ch != '"' && ch != EOF) {
        str += ch;
        current_lexeme += ch;
        get_next_ch();
    }
    if (ch == EOF) error("Незакрытая строка");
    current_lexeme += '"';
    get_next_ch(); // пропускаем " иначе по новой соберем строку
    return Lex(str);
}

Lex Scanner::read_ident_or_keyword() {
    std::string buf;
    while (isalnum(ch) || ch == '_') {
        buf += ch;
        get_next_ch();
    }
    current_lexeme = buf;
    int i = look(buf, TW);
    if (i != 0) {
        // служебное слово
        switch (i) {
            case 1: return Lex(LEX_AND, 1);
            case 2: return Lex(LEX_BEGIN, 2);
            case 3: return Lex(LEX_BOOL, 3);
            case 4: return Lex(LEX_DO, 4);
            case 5: return Lex(LEX_ELSE, 5);
            case 6: return Lex(LEX_END, 6);
            case 7: return Lex(LEX_IF, 7);
            case 8: return Lex(LEX_FALSE, 8);
            case 9: return Lex(LEX_INT, 9);
            case 10: return Lex(LEX_REAL, 10);
            case 11: return Lex(LEX_STRING, 11);
            case 12: return Lex(LEX_NOT, 12);
            case 13: return Lex(LEX_OR, 13);
            case 14: return Lex(LEX_PROGRAM, 14);
            case 15: return Lex(LEX_READ, 15);
            case 16: return Lex(LEX_THEN, 16);
            case 17: return Lex(LEX_TRUE, 17);
            case 18: return Lex(LEX_VAR, 18);
            case 19: return Lex(LEX_WHILE, 19);
            case 20: return Lex(LEX_CONTINUE, 20);
            case 21: return Lex(LEX_WRITE, 21);
            case 22: return Lex(LEX_FIN, 22);
            default: return Lex(LEX_NULL, 0);
        }
    } else {
        int id = put_TID(buf);
        return Lex(LEX_ID, id);
    }
}

Lex Scanner::read_operator() {
    std::string op;
    op += ch;
    get_next_ch();
    if ((op[0] == '=' || op[0] == '!' || op[0] == '<' || op[0] == '>') && ch == '=') {
        op += ch;
        get_next_ch();
    }
    current_lexeme = op;
    int i = look(op, TD);
    if (i == 0) error("Неизвестная лексема");
    switch (i) {
        case 1: return Lex(LEX_COMMA, 1);
        case 2: return Lex(LEX_SEMICOLON, 2);
        case 3: return Lex(LEX_COLON, 3);
        case 4: return Lex(LEX_ASSIGN, 4);
        case 5: return Lex(LEX_LPAREN, 5);
        case 6: return Lex(LEX_RPAREN, 6);
        case 7: return Lex(LEX_EQ, 7);
        case 8: return Lex(LEX_LT, 8);
        case 9: return Lex(LEX_GT, 9);
        case 10: return Lex(LEX_PLUS, 10);
        case 11: return Lex(LEX_MINUS, 11);
        case 12: return Lex(LEX_STAR, 12);
        case 13: return Lex(LEX_SLASH, 13);
        case 14: return Lex(LEX_LE, 14);
        case 15: return Lex(LEX_GE, 15);
        case 16: return Lex(LEX_NE, 16);
        case 17: return Lex(LEX_LBRACE, 17);
        case 18: return Lex(LEX_RBRACE, 18);
        default: return Lex(LEX_NULL,0);
    }
}

Lex Scanner::get_lex() {
    skip_spaces_and_comments();
    if (ch == EOF) {
        current_lexeme = "";
        return Lex(LEX_EOF, 0);
    }
    if (isdigit(ch)) return read_number();
    if (ch == '"') return read_string();
    if (isalpha(ch) || ch == '_') return read_ident_or_keyword();
    return read_operator();
}

void Scanner::error(const std::string& msg) {
    throw std::runtime_error(msg);
}
