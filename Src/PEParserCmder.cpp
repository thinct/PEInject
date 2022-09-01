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

    if (MatchCMD(cmderList[0], QStringList()<<"DisplayImportTable"<<"DIT"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        PrintImportTable(peFilePath);
    }
    if (MatchCMD(cmderList[0], QStringList()<<"DisplayRelocTable"<<"DRT"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        PrintRelocTable(peFilePath);
    }

    return ErrorCode::FAILED;
}

bool PEParserCmder::CheckVaild(QStringList cmderList)
{
    return (cmderList.count() == 2);
}

void PEParserCmder::PrintImportTable(const std::string peFilePath)
{
    PEParser(peFilePath).LoadImportTable();
}

void PEParserCmder::PrintRelocTable(const std::string peFilePath)
{
    PEParser(peFilePath).LoadReloctionTable();
}
