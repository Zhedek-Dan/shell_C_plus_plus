#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <stdexcept>
#include "type_of_lex.h"
#include "ident.h"

class Interpretator {
public:
    Interpretator(const std::vector<Lex>& poliz, std::vector<Ident>& TID): poliz(poliz), TID(TID) {}

    void pusk();

private:
    const std::vector<Lex>& poliz;
    std::vector<Ident>& TID;
    int pc = 0; // счётчик команд (индекс в poliz)
    
    
    
    enum Type { INT, REAL, STRING };
    // стек значений (может хранить int, double, string)
    struct val {
        Type type;
        int i;
        double d;
        std::string s;
    };
    std::stack<val> stack;
    
    // методы работы со стеком
	void push_int(int v) {
	    val object;
	    object.type = INT;
	    object.i = v;
	    stack.push(object);
	}
	void push_real(double v) {
	    val object;
	    object.type = REAL;
	    object.d = v;
	    stack.push(object);
	}
	void push_string(const std::string v) {
	    val object;
	    object.type = STRING;
	    object.s = v;
	    stack.push(object);
	}
	val pop() {
	    if (stack.empty())
			throw std::runtime_error("Стек пуст");
	    val v = stack.top();
	    stack.pop();
	    return v;
	}
	
	
	
	// преобразование типов 
	int to_int(const val& v) const {
		if (v.type == INT)
		    return v.i;
		else if (v.type == REAL)
		    return int(v.d);
		else
		    throw std::runtime_error("Нельзя преобразовать строку в int");
	}
	double to_real(const val& v) const {
		if (v.type == INT)
		    return double(v.i);
		else if (v.type == REAL)
		    return v.d;
		else
		    throw std::runtime_error("Нельзя преобразовать строку в real");
	}
	std::string to_string(const val& v) const {
		if (v.type == INT)
		    return std::to_string(v.i);
		else if (v.type == REAL)
		    return std::to_string(v.d);
		else
		    return v.s;
	}



	// действия
	void do_operation(const Lex& op);
	void do_assign();
	void do_read();
	void do_write();
};

#endif
