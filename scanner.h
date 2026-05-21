#ifndef SCANNER_H
#define SCANNER_H

#include "type_of_lex.h"
#include "ident.h"
#include <cstdio>
#include <vector>
#include <string>

class Scanner {
    FILE* f;
    char ch; // текущий символ
    std::vector<Ident> TID;   // таблица идентификаторов
    std::string current_lexeme;

    void get_next_ch() {
    	ch = fgetc(f);
    }
    void skip_spaces_and_comments();
    int look(const std::string& buf, const char* list[]) { // ищем buf в массиве list
    	int i = 0;
    	while (list[i]){
    		if (buf == list[i])
    			return i;
    		i++;
    	}
    	return 0; // фиктивный 0 в массивах - ""
    }
    Lex read_number(); 
    Lex read_string();
    Lex read_ident_or_keyword();
    Lex read_operator();
    void error(const std::string& msg);

    int put_TID(const std::string& name); // добавляем в массив ид
    int find_TID(const std::string& name); // ищем в массиве ид

public:
    Scanner(const char* program) { 
    	f = fopen(program, "r");
    	if (!f) 
    		throw std::runtime_error("Не можем открыть файл");
    	get_next_ch();
    }
    ~Scanner(){
    	if (f)
    		fclose(f);
    }
    Lex get_lex();
    std::string get_current_lexeme() const { 
    	return current_lexeme; 
    }
    std::vector<Ident>& get_TID() { return TID; }

    static const char* TW[]; // принадлежат всему классу + чтобы не было ошибки
    static const char* TD[];
};

#endif
