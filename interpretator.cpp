#include "interpretator.h"
#include <cmath>
#include <sstream>



void Interpretator::do_operation(const Lex& op) {
    type_of_lex op_type = op.get_type();
    
    if (op_type == LEX_NOT || op_type == LEX_UMINUS) { // одиночные действия
        val v = pop();
        if (op_type == LEX_NOT) {
            if (v.type == 0) 
            	push_int(!(v.i));
            else {
            	throw std::runtime_error("not только к целым");
            }
        } 
        else {
            if (v.type == 0)
            	push_int(-v.i);
            else if (v.type == 1) 
            	push_real(-v.d);
            else 
            	throw std::runtime_error("Унарный минус применим только к числам");
        }
    }
    
     
    else { // бинарные действия
        val right = pop();
        val left = pop();
        if (op_type == LEX_PLUS || op_type == LEX_MINUS || op_type == LEX_STAR || op_type == LEX_SLASH) { // + - * /
        
            if (left.type == 2 && right.type == 2 && op_type == LEX_PLUS) { // у строк только сложение
                push_string(left.s + right.s);
            }
             
            else { // числа
                double l = to_real(left), r = to_real(right);
                double res;
                switch (op_type) {
                    case LEX_PLUS: res = l + r; break;
                    case LEX_MINUS: res = l - r; break;
                    case LEX_STAR: res = l * r; break;
                    case LEX_SLASH:
                        if (r == 0) 
                        	throw std::runtime_error("Деление на ноль");
                        res = l / r; 
                        break;
                    default: throw std::runtime_error("Неизвестная арифметическая операция");
                }
                if (left.type == 0 && right.type == 0 && op_type != LEX_SLASH) // результат целый только если оба целые и не деление
                    push_int((int)res);
                    
                else
                    push_real(res);
            }
        } 
        
        
        else if (op_type == LEX_EQ || op_type == LEX_NE || op_type == LEX_LT || op_type == LEX_GT || op_type == LEX_LE || op_type == LEX_GE) { // сравнение
            bool res = false;
            
            if (left.type == 2 && right.type == 2) { // строки
                if (op_type == LEX_EQ) 
                	res = left.s == right.s;
                else if (op_type == LEX_NE) 
                	res = left.s != right.s;
                else 
                	throw std::runtime_error("Сравнение строк допустимо только == и !=");
            }
             
            else { // числа
                double l = to_real(left), r = to_real(right);
                switch (op_type) {
                    case LEX_EQ: res = (l == r); break;
                    case LEX_NE: res = (l != r); break;
                    case LEX_LT: res = (l < r); break;
                    case LEX_GT: res = (l > r); break;
                    case LEX_LE: res = (l <= r); break;
                    case LEX_GE: res = (l >= r); break;
                    default: break;
                }
            }
            push_int(res ? 1 : 0);
        } 
        
        else if (op_type == LEX_AND || op_type == LEX_OR) { // логические действия
            bool l = (to_int(left) != 0), r = (to_int(right) != 0);
            bool res = (op_type == LEX_AND) ? (l && r) : (l || r);
            push_int(res ? 1 : 0);
        } 
        
        else {
            throw std::runtime_error("Неизвестная операция");
        }
    }
}




void Interpretator:: do_assign() {
    val v = pop();
    val addr = pop();
    if (addr.type != 0) 
    	throw std::runtime_error("Ожидался адрес");
    int idx = addr.i;
    if (idx < 0 || idx >= (int)TID.size()) 
    	throw std::runtime_error("Неверный индекс переменной");
    Ident& id = TID[idx];
    switch (id.get_type()) { // запоминаем значение в поле идентификатора (перезапись идентификатора)
        case LEX_INT: id.set_int(to_int(v)); break;
		case LEX_REAL: id.set_real(to_real(v)); break;	
		case LEX_STRING: id.set_string(to_string(v)); break;
        default: throw std::runtime_error("Неизвестный тип переменной при присваивании");
    }
    stack.push(v);
}

// x = x + 1
// &x x 1 + = ;


void Interpretator::do_read() {
    val addr = pop();
    if (addr.type != 0) 
    	throw std::runtime_error("Ожидался адрес");
    int idx = addr.i;
    Ident& id = TID[idx];
    if (!id.get_declared()) 
    	throw std::runtime_error("Переменная не объявлена");
    std::cout << "Введите значение для " << id.get_name() << ": ";
    switch (id.get_type()) {
        case LEX_INT: {
            int v; std::cin >> v;
            id.set_int(v);
            break;
        }
        case LEX_REAL: {
            double v; std::cin >> v;
            id.set_real(v);
            break;
        }
        case LEX_STRING: {
            std::string v; std::cin >> v;
            id.set_string(v);
            break;
        }
        default: throw std::runtime_error("Неизвестный тип переменной при чтении");
    }
}
// read (x);
// &x read ;




void Interpretator::do_write() {
    val v = pop();
    switch (v.type) {
        case 0: std::cout << v.i << std::endl; break;
        case 1: std::cout << v.d << std::endl; break;
        case 2: std::cout << v.s << std::endl; break;
    }
}
// write (x);
// x write;




void Interpretator::pusk() {
    pc = 0;
    while (pc < (int)poliz.size()) {
        const Lex& curr = poliz[pc];
        // std::cerr << "pc=" << pc << " cmd=" << Lex::lex_type_names[curr.get_type()] << std::endl;
        switch (curr.get_type()) {
            case LEX_INT_val:      push_int(curr.get_value()); break;
            case LEX_REAL_val:     push_real(curr.get_real()); break;
            case LEX_STRING_val:   push_string(curr.get_string()); break;
            case LEX_ID: { // передаем идентификатор (не для присвоения) как его значение, чтобы работать с ним (должен быть не пустым)
                int idx = curr.get_value();
                if (idx < 0 || idx >= (int)TID.size() || !TID[idx].get_declared())
                    throw std::runtime_error("Переменная не объявлена");
                Ident& id = TID[idx];
                if (!id.get_assigned()) 
                	throw std::runtime_error("Переменная пустая");
                switch (id.get_type()) {
                    case LEX_INT: push_int(id.get_int()); break;
                    case LEX_REAL: push_real(id.get_real()); break;
                    case LEX_STRING: push_string(id.get_string()); break;
                    default: throw std::runtime_error("Неизвестный тип переменной");
                }
                break;
            }
            case POLIZ_ADDR:       push_int(curr.get_value()); break; // передаем идентификатор как индекс TID, чтобы переприсвоить
            case LEX_ASSIGN:       do_assign(); break;
            case POLIZ_READ:       do_read(); break;
            case POLIZ_WRITE:      do_write(); break;
            case LEX_SEMICOLON: // удаляем ненужное значение   
            	if (!stack.empty()) 
            		stack.pop(); 
            	break;
            case POLIZ_GO:         
            	//std::cerr << "  -> переход на " << curr.get_value() << std::endl;
            	pc = curr.get_value() - 1; break; // -1 т.к. всегда заканчивам ++pc
            	break;
            case POLIZ_FGO:        
            	if (to_int(stack.top()) == 0)  { // условие неверное 
            			pc = curr.get_value() - 1; // -1 т.к. в конце цикла ++pc
            	} 
            	stack.pop(); 
            	break;
            case LEX_PLUS: case LEX_MINUS: case LEX_STAR: case LEX_SLASH:
            case LEX_EQ: case LEX_NE: case LEX_LT: case LEX_GT:
            case LEX_LE: case LEX_GE: case LEX_AND: case LEX_OR:
            case LEX_NOT: case LEX_UMINUS:
                do_operation(curr);
                break;
            default: break;
        }
        ++pc;
    }
}
