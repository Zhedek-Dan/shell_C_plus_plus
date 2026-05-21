#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include <stack>
#include <iostream>
#include <stdexcept>
#include <vector>

class Parser {
	Scanner scanner;
	Lex curr_lex;
	type_of_lex curr_lex_type;
	int curr_lex_val;
	
	bool left_is_id = false;
	
	std::stack<int> st_indx; // стек индексов идентификаторов - используем только в инициализации переменных
	std::stack<type_of_lex> st_lex; // стек типов лексем
	std::vector<Ident>& TID;

        // ПОЛИЗ
        std::vector<Lex> poliz;
        std::stack<int> loop_addrs;
        
        std::vector<int> do_while; 
        std::stack<bool> in_do;

	
	void next_lex() {
		curr_lex = scanner.get_lex();
		curr_lex_type = curr_lex.get_type();
		curr_lex_val = curr_lex.get_value();
	}
	void error(const std::string& msg){
		throw std::runtime_error(msg);
	}
	void Program();
	void Declarations();
	void Declaration();
	void Type();
	void IdentList(type_of_lex type);
	type_of_lex ConstExpr();
	void Statements();
	void Statement();
	
	void Expr();        // точка входа
	void Assign();      // =
	void OrExpr();      // or
	void AndExpr();     // and
	void RelExpr();     // сравнения
	void AddExpr();     // + -
	void MulExpr();     // * /
	void UnaryExpr();   // - not
	void Primary();     // ID, константы и тд
	
	void dec(type_of_lex type);
	void check_id();
	void check_op();
	void check_not();
	void check_uminus();
	void eq_type();
	void check_bool_expr();
	
    static type_of_lex val_to_type(type_of_lex t) {
        switch (t) {
            case LEX_INT_val:    return LEX_INT;
            case LEX_REAL_val:   return LEX_REAL;
            case LEX_STRING_val: return LEX_STRING;
            default:             return LEX_NULL;
        }
    }
    
	
public:
	Parser(const char* filename): scanner(filename), TID(scanner.get_TID()) {}
	void analyze();
	std::vector<Ident>& take_TID() { return TID; }
	std::vector<Lex>& take_poliz() { return poliz; }
};

#endif
