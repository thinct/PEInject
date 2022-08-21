#include <QCoreApplication>
#include <iostream>
#include "Tester/FileTest.hpp"
#include "Src/PEParser.h"

using namespace PEInjector;

int main(int argc, char *argv[])
{
    // system("pause");

    if (argc > 1) {
        PEParser parser(argv[1]);
        parser.loadImportTable();
    }
    else {
        std::cout << "Nothing to parse" << std::endl;
    }

    return 0;
}
