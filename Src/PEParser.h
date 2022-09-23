#ifndef __PE_PARSER_HPP__
#define __PE_PARSER_HPP__

#include <iostream>


namespace PEInjector
{

class PEParser
{
public:
    PEParser(const std::string& pePath);

    PEParser&     ReadHeadersAndSections();
    PEParser&     LoadImportTable();
    PEParser&     LoadReloctionTable();
    PEParser&     RebuildImportTable(std::string tagName, std::uint32_t fileOffset);
    PEParser&     RebuildRelocTable(std::string tagName, std::uint32_t fileOffset);
    PEParser&     InjectDll(std::string moduleName, std::string funcName);
    PEParser&     InjectCode(std::uint32_t rvaHook, std::uint8_t lenCode, std::string tagName, std::uint32_t fileOffset);
    PEParser&     AppendNewSection(std::string tagName, uint32_t size, uint32_t characteristics);

    std::uint32_t RvaToFileOffset(std::uint32_t dwRva);
    void          SaveAs(const std::string& pePath);

private:
    friend class PEParserPrivate;
    class PEParserPrivate* m_pData;
};
}

#endif // __PE_PARSER_HPP__

