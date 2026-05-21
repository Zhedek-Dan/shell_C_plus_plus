#include "interpretator.h"
#include "parser.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <файл>" << std::endl;
        return 1;
    }
    try {
        Parser parser(argv[1]);
        parser.analyze();
        Interpretator interpretator(parser.take_poliz(), parser.take_TID());
        interpretator.pusk();
    } 
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
