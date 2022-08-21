#ifndef __PE_PARSER_HPP__
#define __PE_PARSER_HPP__

#include <iostream>


namespace PEInjector
{

class PEParser
{
public:
    PEParser(const std::string& pePath);

    void  readHeadersAndSections(const std::string& pePath);
    void  loadImportTable();

private:
    friend class PEParserPrivate;
    class PEParserPrivate* m_pData;
};
}

#endif // __PE_PARSER_HPP__

