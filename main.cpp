#include <QCoreApplication>
#include "Cmder/CommandLineParse.hpp"
#include "Src/CmderParseHeaders.h"

int main(int argc, char *argv[])
{
    // system("pause");

    Cmder::CommandLineParse::GetInstance()
            ->InitMainEntryPointParams(argc, argv)
            .exec();

    return 0;
}
