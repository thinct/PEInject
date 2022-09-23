#include "PEParserCmder.h"
#include <QDir>
#include <QFileInfo>
#include "Src/PEParser.h"
#include "Cmder/CommandLineParse.hpp"

using namespace Cmder;
using namespace PEInjector;


REGIST_CMDER_OBJ(PEParserCmder)

ICmder::ErrorCode PEParserCmder::ChangeEvent(QStringList cmderList)
{
    switch (cmderList.count())
    {
    case 2:
        goto LABEL_CASE_2;
    case 4:
        goto LABEL_CASE_4;
    }
    goto CASE_END;
    

LABEL_CASE_2:
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

LABEL_CASE_4:
    if (MatchCMD(cmderList[0], QStringList() << "SimpleInject"<<"Inject"))
    {
        const std::string peFilePath = cmderList[1].remove("\"").toStdString();
        QString addrInjected = cmderList[2];
        std::uint32_t uAddrInjected = addrInjected.toLower().remove("0x").toUInt();
        SimpleInject(peFilePath, uAddrInjected, cmderList[3].toUInt());
        std::cout << "\n=========================================\n\n";
        return ErrorCode::SUCCESS;
    }


CASE_END:
    return ErrorCode::FAILED;
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

void PEParserCmder::SimpleInject(const std::string peFilePath, std::uint32_t injectAddr, std::uint16_t instructsLen)
{
    QFileInfo fileInfo = QFileInfo(peFilePath.data());
    QString fileDir = fileInfo.absoluteDir().path();
    QString fileName = fileInfo.baseName() + "-Injected." + fileInfo.suffix();
    std::string newInjectedFilePath = QString(fileDir + "/" + fileName).toStdString();

    PEParser(peFilePath)
        .ReadHeadersAndSections()
        .LoadImportTable()
        .AppendNewSection(".rdata2", 0x6000, 0x40000040)
        .AppendNewSection(".text2", 0x1000, 0x60000020)
        .RebuildImportTable(".rdata2", 0x0)
        .InjectDll("Inject.dll", "INT")
        .InjectCode(injectAddr, instructsLen, ".text2", 0x0)
        .RebuildRelocTable(".rdata2", 0x5000)
        .SaveAs(newInjectedFilePath);
}
