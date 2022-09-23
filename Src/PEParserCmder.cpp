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
        return ErrorCode::SUCCESS;
    }
    if (MatchCMD(cmderList[0], QStringList()<<"DisplayRelocTable"<<"DRT"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        PrintRelocTable(peFilePath);
        return ErrorCode::SUCCESS;
    }
    if (MatchCMD(cmderList[0], QStringList() << "AddNewSection" << "ANS"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        AddNewSection(peFilePath);
        return ErrorCode::SUCCESS;
    }
    if (MatchCMD(cmderList[0], QStringList() << "SaveAs"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        SaveAs(peFilePath);
        return ErrorCode::SUCCESS;
    }

    return ErrorCode::FAILED;
}

bool PEParserCmder::CheckVaild(QStringList cmderList)
{
    return (cmderList.count() == 2);
}

void PEParserCmder::PrintImportTable(const std::string peFilePath)
{
    PEParser(peFilePath).ReadHeadersAndSections().LoadImportTable();
}

void PEParserCmder::PrintRelocTable(const std::string peFilePath)
{
    PEParser(peFilePath).ReadHeadersAndSections().LoadReloctionTable();
}

void PEParserCmder::AddNewSection(const std::string peFilePath)
{
    PEParser(peFilePath).ReadHeadersAndSections().AppendNewSection(".newsec", 0x3000, 0);
}

void PEParserCmder::SaveAs(const std::string peFilePath)
{
    // PEParser(peFilePath).ReadHeadersAndSections().SaveAs("E:/1.exe");
    PEParser(peFilePath)
        .ReadHeadersAndSections()
        .LoadImportTable()
        .AppendNewSection(".rdata2", 0x6000, 0x40000040)
        .AppendNewSection(".text2", 0x1000, 0x60000020)
        .RebuildImportTable(".rdata2", 0x0)
        .InjectDll("Inject.dll", "INT")
        .InjectCode(0x123, 5, ".text2", 0x0)
        .RebuildRelocTable(".rdata2", 0x5000)
        .SaveAs("E:/2.exe");
}
