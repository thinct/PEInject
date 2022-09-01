#ifndef PEPARSERPRIVATE_H
#define PEPARSERPRIVATE_H

#include <map>
#include "PEStruct.h"
#include "Global/BinaryFile.hpp"

using namespace PEStruct;
using namespace FileSystem;


namespace PEInjector
{

class PEParser;

class PEParserPrivate
{
private:
    PEParserPrivate(PEParser* pPEParser);

private:
    char* addr(std::uint32_t rva);
    bool  checkDOSHeader();
    bool  checkPESignature();

private:
    template <typename T>
    void  readSection(std::uint32_t rva, T& t)
    {
        // Looks for the right section.
        memset(&t, 0, sizeof(T));
        char* addrRet = addr(rva);
        if (addrRet)
        {
            t = *reinterpret_cast<T*>(addrRet);
        }
    }

private:
    friend class PEParser;
    class PEParser* m_pData;

    DOSHeader                                       m_dosHeader;
    std::vector<unsigned char>                      m_dosStub;
    COFFHeader                                      m_coffHeader;
    PEOptHeader                                     m_peOptHeader;
    std::vector<SectionHeader>                      m_sectionHeaders;
    std::vector<unsigned char>                      m_padding;
    std::vector<std::vector<unsigned char>>         m_sections;
    std::vector<DllImport>                          m_importedDlls;
    std::map<uint32_t, std::vector<ImageRelocItem>> m_relocTables;
};

}


#endif // PEPARSERPRIVATE_H
