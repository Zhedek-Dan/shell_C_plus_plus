#ifndef TYPE_OF_LEX_H
#define TYPE_OF_LEX_H

#include <iostream>
#include <string>

enum type_of_lex {
    LEX_NULL = 0,
    
    // служебные слова
    LEX_AND, LEX_BEGIN, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_END,
    LEX_IF, LEX_FALSE, LEX_INT, LEX_REAL, LEX_STRING, LEX_NOT, LEX_OR,
    LEX_PROGRAM, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR,
    LEX_WHILE, LEX_CONTINUE, LEX_WRITE, LEX_FIN,
    
    // разделители
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN,
    LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LT, LEX_GT,
    LEX_PLUS, LEX_MINUS, LEX_STAR, LEX_SLASH, LEX_LE, LEX_GE, LEX_NE,
    LEX_LBRACE, LEX_RBRACE,
    
    // константы и идентификатор
    LEX_INT_val,
    LEX_REAL_val,
    LEX_STRING_val,
    LEX_ID,
    
    // ПОЛИЗ
    POLIZ_ADDR,  // адрес переменной
    POLIZ_GO,    // безусловный переход
    POLIZ_FGO,   // переход по false
    POLIZ_READ,  // чтение
    POLIZ_WRITE, // запись  
    LEX_UMINUS,

    LEX_EOF
};


class Lex {
    type_of_lex t_lex;
    int v_lex;               // для int значение, для TW/TD/TID - номер в таблице
    double real_val;         // значение для real
    std::string str_val;     // значение для string
public:
	// конструкторы
    Lex(type_of_lex t = LEX_NULL, int v = 0): t_lex(t), v_lex(v), real_val(0) {} // общий случай
    
    Lex(int val): t_lex(LEX_INT_val), v_lex(val), real_val(0) {} // конструктор для int константы
  
    Lex(double val): t_lex(LEX_REAL_val), v_lex(0), real_val(val) {} // для real константы

    Lex(const std::string& val): t_lex(LEX_STRING_val), v_lex(0), str_val(val) {} // для string константы

    type_of_lex get_type() const { 
    	return t_lex; 
    }
    int get_value() const { 
    	return v_lex; 
    }
    double get_real() const { 
    	return real_val; 
    }
    std::string get_string() const { 
    	return str_val; 
    }
    
    friend std::ostream& operator<<(std::ostream& s, const Lex& lex) {
    	s << '(' << Lex::lex_type_names[lex.t_lex] << ',';
    	switch (lex.t_lex) {
        	case LEX_REAL_val:  s << lex.real_val; break;
        	case LEX_STRING_val: s << '"' << lex.str_val << '"'; break;
        	default:            s << lex.v_lex;
    	}
    	s << ')';
    	return s;
	}
	
	static const char* lex_type_names[]; // используем static, чтобы не было множественного определения + общий массив для всего класса
};

#endif
