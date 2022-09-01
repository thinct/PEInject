#ifndef __PE_PARSER_HPP__
#define __PE_PARSER_HPP__

#include <iostream>


namespace PEInjector
{

class PEParser
{
public:
    PEParser(const std::string& pePath);

    void          ReadHeadersAndSections(const std::string& pePath);
    void          LoadImportTable();
    void          LoadReloctionTable();
    std::uint32_t RvaToFileOffset(std::uint32_t dwRva);

private:
    friend class PEParserPrivate;
    class PEParserPrivate* m_pData;
};
}

#endif // __PE_PARSER_HPP__

