#ifndef IDENT_H
#define IDENT_H

#include <string>
#include "type_of_lex.h"

class Ident {
    std::string name;
    bool declared;
    type_of_lex type;
    bool assigned;

    // значения переменных (для интерпретатора)
    int int_val = 0;
    double real_val = 0.0;
    std::string str_val;

public:
    Ident() : declared(false), assigned(false), type(LEX_NULL) {}
    Ident(const std::string& n) : name(n), declared(false), assigned(false), type(LEX_NULL) {}

    const std::string& get_name() const { return name; }
    bool get_declared() const { return declared; }
    void set_declared() { declared = true; }
    type_of_lex get_type() const { return type; }
    void set_type(type_of_lex t) { type = t; }
    bool get_assigned() const { return assigned; }
    void set_assigned() { assigned = true; }

    // методы для значений
    int get_int() const { return int_val; }
    void set_int(int v) { int_val = v; assigned = true; }
    double get_real() const { return real_val; }
    void set_real(double v) { real_val = v; assigned = true; }
    std::string get_string() const { return str_val; }
    void set_string(const std::string& v) { str_val = v; assigned = true; }
};

#endif
