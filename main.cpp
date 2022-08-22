#include <QCoreApplication>
#include "CommandLineParse.h"

int main(int argc, char *argv[])
{
    // system("pause");

    CommandLineParse cmdParse(argc, argv);
    cmdParse.exec();

    return 0;
}
