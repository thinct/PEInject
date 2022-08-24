#include "PEParserCmder.h"
#include "Src/PEParser.h"
#include "Cmder/CommandLineParse.hpp"

using namespace Cmder;
using namespace PEInjector;


REGIST_CMDER_OBJ(PEParserCmder)

ICmder::ErrorCode PEParserCmder::ChangeEvent(QStringList cmderList)
{
    if (!CheckVaild(cmderList))
    {
        return ErrorCode::FAILED;
    }

    const std::string peFilePath = cmderList[1].remove("\"").toStdString();
    PEParser parser(peFilePath);
    parser.loadImportTable();

    return ErrorCode::SUCCESS;
}

bool PEParserCmder::CheckVaild(QStringList cmderList)
{
    return (cmderList.count() == 2
            && MatchCMD(cmderList[0], QStringList()<<"DisplayImportTable"<<"DIT"));
}
