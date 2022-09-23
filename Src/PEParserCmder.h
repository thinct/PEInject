#ifndef PEPARSERCMDER_H
#define PEPARSERCMDER_H

#include "Cmder/ICmder.h"
class PEParserCmder : public Cmder::ICmder
{
protected:
    virtual ErrorCode ChangeEvent(QStringList cmderList) override;

private:
    void PrintImportTable(const std::string peFilePath);
    void PrintRelocTable(const std::string peFilePath);
    void AddNewSection(const std::string peFilePath);
    void SimpleInject(const std::string peFilePath, std::uint32_t injectAddr, std::uint16_t instructsLen);
};

#endif // PEPARSERCMDER_H
