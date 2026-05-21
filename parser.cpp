#include "parser.h"

void Parser::analyze() {
    next_lex();
    Program();
    if (curr_lex_type != LEX_EOF)
        error("Лишний текст после конца программы");
    std::cout << "Синтаксический анализ завершен успешно.\n";
    for (const auto& l : poliz)
        std::cout << l << ' ';
    std::cout << std::endl;
}

void Parser::Program() { // program { Declarations Statements }
    if (curr_lex_type != LEX_PROGRAM)
        error("Ожидалась 'program'");
    next_lex();
    if (curr_lex_type != LEX_LBRACE)
        error("Ожидалась '{' после 'program'");
    next_lex();
    Declarations();
    Statements();
    if (curr_lex_type != LEX_RBRACE)
        error("Ожидалась '}' в конце программы");
    next_lex();
}

void Parser::Declarations() {
    while (curr_lex_type == LEX_INT || curr_lex_type == LEX_REAL || curr_lex_type == LEX_STRING) {
        Declaration();
        if (curr_lex_type != LEX_SEMICOLON)
            error("Ожидалась ';' после описания");
        next_lex();
    }
}

void Parser::Declaration() {
    type_of_lex s = curr_lex_type;
    Type();
    IdentList(s);
    dec(s);
}

void Parser::Type() {
    if (curr_lex_type == LEX_INT || curr_lex_type == LEX_STRING || curr_lex_type == LEX_REAL) {
        next_lex();
    } else {
        error("Ожидался тип (int, real, string)");
    }
}

void Parser::IdentList(type_of_lex id_type) {
    if (curr_lex_type != LEX_ID)
        error("Ожидался идентификатор"); // вначале должен идти ID
    st_indx.push(curr_lex_val);
    next_lex();

    if (curr_lex_type == LEX_ASSIGN) {
        next_lex();
        type_of_lex const_type = curr_lex_type;
        Lex saved_const = curr_lex;
        next_lex();
        if (val_to_type(const_type) != id_type)
            error("Тип константы не соответствует типу переменной");
        int idx = st_indx.top();
        poliz.push_back(Lex(POLIZ_ADDR, idx));
        poliz.push_back(saved_const);
        poliz.push_back(Lex(LEX_ASSIGN));
        poliz.push_back(Lex(LEX_SEMICOLON));
    }

    // последующие ID
    while (curr_lex_type == LEX_COMMA) {
        next_lex(); // пропускаем запятую
        if (curr_lex_type != LEX_ID)
            error("Ожидался идентификатор");
        st_indx.push(curr_lex_val);
        next_lex();
        if (curr_lex_type == LEX_ASSIGN) {
            next_lex();
            type_of_lex const_type = curr_lex_type;
            Lex saved_const = curr_lex;
            next_lex();
            if (val_to_type(const_type) != id_type)
                error("Тип константы не соответствует типу переменной");
            int idx = st_indx.top();
            poliz.push_back(Lex(POLIZ_ADDR, idx));
            poliz.push_back(saved_const);
            poliz.push_back(Lex(LEX_ASSIGN));
            poliz.push_back(Lex(LEX_SEMICOLON));
        }
    }
}

type_of_lex Parser::ConstExpr() {
    type_of_lex c = curr_lex_type;
    if (c == LEX_INT_val || c == LEX_REAL_val || c == LEX_STRING_val) {
        poliz.push_back(curr_lex);
        next_lex();
        return c;
    } else {
        error("Ожидалась константа");
    }
    return LEX_NULL;
}


void Parser::Expr() { // от низшего к высшему 
    Assign();
}

void Parser::Assign() { // присваивание - самый низкий приоритет
    int left_pos = poliz.size();
    OrExpr(); // здесь добавили идентификатор в полиз
    if (curr_lex_type == LEX_ASSIGN) {
        if (!left_is_id)
            error("Левая часть присваивания должна быть идентификатором");
        if (poliz.size() != left_pos + 1 || poliz.back().get_type() != LEX_ID)
            error("Левая часть присваивания должна быть одиночным идентификатором");
        int idx = poliz.back().get_value();
        poliz.back() = Lex(POLIZ_ADDR, idx); // заменили на адрес !!!
        next_lex();
        Assign();   // правоассоциативность, доходим рекурсией до конца
        eq_type();
        poliz.push_back(Lex(LEX_ASSIGN));
    }
}

void Parser::OrExpr() {
    AndExpr();
    while (curr_lex_type == LEX_OR) { // левоасоцитивность, идем циклом 
        type_of_lex op = curr_lex_type;
        st_lex.push(op);
        next_lex();
        AndExpr();
        check_op();
        poliz.push_back(Lex(op));
    }
}

void Parser::AndExpr() {
    RelExpr();
    while (curr_lex_type == LEX_AND) {
        type_of_lex op = curr_lex_type;
        st_lex.push(op);
        next_lex();
        RelExpr();
        check_op();
        poliz.push_back(Lex(op));
    }
}

void Parser::RelExpr() {
    AddExpr();
    if (curr_lex_type == LEX_EQ || curr_lex_type == LEX_NE || curr_lex_type == LEX_LT ||
        curr_lex_type == LEX_GT || curr_lex_type == LEX_LE || curr_lex_type == LEX_GE) {
        type_of_lex op = curr_lex_type;
        st_lex.push(op);
        next_lex();
        AddExpr();
        check_op();
        poliz.push_back(Lex(op));
    }
}

void Parser::AddExpr() {
    MulExpr();
    while (curr_lex_type == LEX_PLUS || curr_lex_type == LEX_MINUS) {
        type_of_lex op = curr_lex_type;
        st_lex.push(op);
        next_lex();
        MulExpr();
        check_op();
        poliz.push_back(Lex(op));
    }
}

void Parser::MulExpr() {
    UnaryExpr();
    while (curr_lex_type == LEX_STAR || curr_lex_type == LEX_SLASH) {
        type_of_lex op = curr_lex_type;
        st_lex.push(op);
        next_lex();
        UnaryExpr();
        check_op();
        poliz.push_back(Lex(op));
    }
}

void Parser::UnaryExpr() {
    if (curr_lex_type == LEX_NOT){
        next_lex();
        UnaryExpr();
        check_not();
        poliz.push_back(Lex(LEX_NOT));
    } 
    else if (curr_lex_type == LEX_MINUS) {
        next_lex();
        UnaryExpr();
        check_uminus();
        poliz.push_back(Lex(LEX_UMINUS));
    }
    else {
        Primary();
    }
}

void Parser::Primary() {
    switch (curr_lex_type) {
        case LEX_ID:
            check_id();
            poliz.push_back(Lex(LEX_ID, curr_lex_val));
            left_is_id = true;
            next_lex();
            break;
        case LEX_INT_val:
            st_lex.push(LEX_INT);
            poliz.push_back(curr_lex);
            left_is_id = false;
            next_lex();
            break;
        case LEX_REAL_val:
            st_lex.push(LEX_REAL);
            poliz.push_back(curr_lex);
            left_is_id = false;
            next_lex();
            break;
        case LEX_STRING_val:
            st_lex.push(LEX_STRING);
            poliz.push_back(curr_lex);
            left_is_id = false;
            next_lex();
            break;
        case LEX_LPAREN:
            left_is_id = false;
            next_lex();
            Expr();
            if (curr_lex_type != LEX_RPAREN)
                error("Ожидалась ')'");
            next_lex();
            break;
        default:
            error("Ожидался операнд (идентификатор, число, строка или '(')");
    }
}

void Parser::Statements() {
    while (curr_lex_type != LEX_RBRACE && curr_lex_type != LEX_EOF && curr_lex_type != LEX_ELSE) { // Лексемы начинающие оператор
        Statement();
    }
}

void Parser::Statement() { // Expr - выражение, Statement - оператор
    switch (curr_lex_type) {
        case LEX_LBRACE: {         
            next_lex();
            Statements();
            if (curr_lex_type != LEX_RBRACE)
                error("Ожидалась '}'");
            next_lex();
            break;
        }
        
        case LEX_IF: { // if (Expr) Statement else Statement
            next_lex();
            if (curr_lex_type != LEX_LPAREN)
                error("Ожидалась '(' после 'if'");
            next_lex();
            Expr(); // разбираем условие
            check_bool_expr();
            if (curr_lex_type != LEX_RPAREN)
                error("Ожидалась ')' после условия");
            next_lex();
            int false_label = poliz.size();
            poliz.push_back(Lex(POLIZ_FGO, 0)); // !F
            Statement(); // разбираем оператор1
            int end_label = poliz.size();
            poliz.push_back(Lex(POLIZ_GO, 0)); // F
            poliz[false_label] = Lex(POLIZ_FGO, (int)poliz.size()); // заполнили !F
            if (curr_lex_type != LEX_ELSE)
                error("Ожидалось 'else' после блока 'if'");
            next_lex();
            Statement(); // разбираем оператор2
            poliz[end_label] = Lex(POLIZ_GO, (int)poliz.size());
            break;
        }
         
        case LEX_DO: { // проблема - continue в теле
  			next_lex();
  			in_do.push(true);
  			int help = do_while.size();
  			int do_start = poliz.size();
    		Statement();                         // тело
    		if (curr_lex_type != LEX_WHILE)
        		error("Ожидалась 'while' после 'do'");
    		next_lex();
    		if (curr_lex_type != LEX_LPAREN)
        		error("Ожидалась '(' после 'while'");
    		next_lex();
    		int loop_start = poliz.size();       // начало условия
    		Expr();                              // условие
    		check_bool_expr();
    		if (curr_lex_type != LEX_RPAREN)
        		error("Ожидалась ')' после условия");
    		next_lex();
    		int exit_label = poliz.size();
    		poliz.push_back(Lex(POLIZ_FGO, 0));  // выход при !F
    		poliz.push_back(Lex(POLIZ_GO, do_start)); // обратно на do
    		poliz[exit_label] = Lex(POLIZ_FGO, (int)poliz.size()); // выход
    		
    		for (int i = help; i < (int)do_while.size(); ++i) {
        		poliz[do_while[i]] = Lex(POLIZ_GO, loop_start); // исправили
    		}
    		
    		do_while.resize(help);
    		in_do.pop();
    		
    		if (curr_lex_type != LEX_SEMICOLON)
        		error("Ожидалась ';' после 'do-while'");
    		next_lex();
    		break;
}
        
        case LEX_WHILE: { // while ( Expr ) Statement
            next_lex();
            if (curr_lex_type != LEX_LPAREN)
                error("Ожидалась '(' после 'while'");
            next_lex();
            int cond_addr = poliz.size();
            Expr(); // разбираем условие
            check_bool_expr();
            if (curr_lex_type != LEX_RPAREN)
                error("Ожидалась ')' после условия");
            next_lex();
            int false_jump = poliz.size();
            poliz.push_back(Lex(POLIZ_FGO, 0));
            loop_addrs.push(cond_addr);
            Statement(); // разбираем тело цикла
            poliz.push_back(Lex(POLIZ_GO, cond_addr));
            poliz[false_jump] = Lex(POLIZ_FGO, (int)poliz.size());
            loop_addrs.pop();
            break;
        }
        
        case LEX_READ: { // read ( ID ) ;
            next_lex();
            if (curr_lex_type != LEX_LPAREN) 
                error("Ожидалась '(' после 'read'");
            next_lex(); 
            if (curr_lex_type != LEX_ID) 
                error("Ожидался идентификатор");
            int idx = curr_lex_val;
            if (!TID[idx].get_declared())
                error("Переменная в read не объявлена");
            poliz.push_back(Lex(POLIZ_ADDR, idx));
            poliz.push_back(Lex(POLIZ_READ));
            next_lex(); 
            if (curr_lex_type != LEX_RPAREN) 
                error("Ожидалась ')' после идентификатора");
            next_lex(); 
            if (curr_lex_type != LEX_SEMICOLON) 
                error("Ожидалась ';' после оператора 'read'");
            next_lex();
            break;
        }

        case LEX_WRITE: { // write ( Expr { , Expr } ) ;
            next_lex();                  
            if (curr_lex_type != LEX_LPAREN) 
                error("Ожидалась '(' перед списком выражений");
            next_lex();  
            Expr();
            st_lex.pop();
            poliz.push_back(Lex(POLIZ_WRITE));
            while (curr_lex_type == LEX_COMMA) {
                next_lex();
                Expr();
                st_lex.pop();
                poliz.push_back(Lex(POLIZ_WRITE));
            }
            if (curr_lex_type != LEX_RPAREN) 
                error("Ожидалась ')' после списка выражений");
            next_lex();
            if (curr_lex_type != LEX_SEMICOLON)
                error("Ожидалась ';' после оператора 'write'");
            next_lex();
            break;
        }
        case LEX_CONTINUE: { // continue ;
            next_lex();
            if (curr_lex_type != LEX_SEMICOLON) 
                error("Ожидалась ';' после оператора 'continue'");
            if (!in_do.empty() && in_do.top()) {
            	int pos = poliz.size();
            	poliz.push_back(Lex(POLIZ_GO,0)); // заглушка
            	do_while.push_back(pos); // запомнили индекс команды полиза
            } 
            else if (!loop_addrs.empty()) {
            	
            	int target = loop_addrs.top();
            	poliz.push_back(Lex(POLIZ_GO, target));
            }
            else {
            	error("continue ожидается в цикле (do/while)");
            }
            next_lex();
            break;
        }
        default: {  // Expr ; - оператор-выражение
            Expr();
            st_lex.pop();
            poliz.push_back(Lex(LEX_SEMICOLON));
            if (curr_lex_type != LEX_SEMICOLON)
                error("Ожидалась ';' после оператора-выражения");
            next_lex();
            break;
        }
    }
}

void Parser::dec(type_of_lex lex_type) { // объявляем переменные одного типа
    while (!st_indx.empty()) {
        int indx = st_indx.top();
        st_indx.pop();
        Ident& id = TID[indx];
        if (id.get_declared())
            error("Повторное объявление идентификатора");
        id.set_declared();
        id.set_type(lex_type); // меняем LEX_ID на соответствующий тип идентификатора (int, real, stirng)
    }
}

void Parser::check_id() { // проверяем на объявленность + сохраняем тип лексемы в стек
    int indx = curr_lex_val;
    if (indx < 0 || indx >= (int)TID.size() || !TID[indx].get_declared())
        error("Использование необъявленного идентификатора");
    st_lex.push(TID[indx].get_type());
}

void Parser::check_op() { // проверяем бинарный оператор
    type_of_lex right = st_lex.top(); st_lex.pop();
    type_of_lex op    = st_lex.top(); st_lex.pop();
    type_of_lex left  = st_lex.top(); st_lex.pop();
    type_of_lex result;

    if (op == LEX_PLUS && left == LEX_STRING && right == LEX_STRING) { // строки - только сложение
        result = LEX_STRING;
    }
    else if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_STAR || op == LEX_SLASH) { // арифм операции
        if ((left == LEX_INT || left == LEX_REAL) && (right == LEX_INT || right == LEX_REAL)) { // числа
            result = (left == LEX_REAL || right == LEX_REAL) ? LEX_REAL : LEX_INT;
        } else {
            error("Несовместимые типы для арифметической операции");
        }
    } else if (op == LEX_EQ || op == LEX_NE || op == LEX_LT || op == LEX_GT || // сравнение
               op == LEX_LE || op == LEX_GE) { 
        if (left == right && (left == LEX_INT || left == LEX_REAL || left == LEX_STRING))
            result = LEX_INT;   // результат сравнения всегда int
        else
            error("Несовместимые типы для операции сравнения");
    } else if (op == LEX_AND || op == LEX_OR) { // логические
        if (left == LEX_INT && right == LEX_INT)
            result = LEX_INT;
        else
            error("Операнды and/or должны быть целыми");
    } else {
        error("Внутренняя ошибка в check_op");
    }
    
    st_lex.push(result);
}

void Parser::check_not() {
    if (st_lex.top() != LEX_INT)
        error("not применяется только к целому типу");
}

void Parser::check_uminus() {
    type_of_lex t = st_lex.top();
    if (t != LEX_INT && t != LEX_REAL)
        error("Унарный минус применим только к числам");
}

void Parser::eq_type() {
    type_of_lex right = st_lex.top(); st_lex.pop();
    type_of_lex left  = st_lex.top(); st_lex.pop();

    if (left == LEX_INT && right == LEX_REAL) {}
    else if (left == LEX_REAL && right == LEX_INT) {}
    else if (left != right)
        error("Несовместимые типы в присваивании");
    st_lex.push(left);
}

void Parser::check_bool_expr() {
    if (st_lex.top() != LEX_INT)
        error("Условие должно быть целым выражением");
    st_lex.pop();
}
