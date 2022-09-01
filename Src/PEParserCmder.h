#ifndef PEPARSERCMDER_H
#define PEPARSERCMDER_H

#include "Cmder/ICmder.h"
class PEParserCmder : public Cmder::ICmder
{
protected:
    virtual ErrorCode ChangeEvent(QStringList cmderList) override;
    virtual bool      CheckVaild(QStringList cmderList) override;

private:
    void PrintImportTable(const std::string peFilePath);
    void PrintRelocTable(const std::string peFilePath);
};

#endif // PEPARSERCMDER_H
