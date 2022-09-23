#ifndef PEPARSERPRIVATE_H
#define PEPARSERPRIVATE_H

#include <map>
#include <assert.h>
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
    PEParserPrivate(PEParser* pPEParser, const std::string filePath);

private:
    char*                       addr(std::uint32_t rva);
    bool                        checkDOSHeader();
    bool                        checkPESignature();
    uint32_t                    align(std::uint32_t ad, std::uint32_t alignment);
    std::vector<unsigned char>& section(std::string tagName, bool* ok=nullptr);
    SectionHeader&              sectionHeader(std::string tagName, bool* ok = nullptr);

private:
    template <typename T>
    void     readSection(std::uint32_t rva, T& t)
    {
        // Looks for the right section.
        memset(&t, 0, sizeof(T));
        char* addrRet = addr(rva);
        if (addrRet)
        {
            t = *reinterpret_cast<T*>(addrRet); // unsafe, may be out of memory
        }
    }

    template <typename T>
    void     writeSection(std::uint32_t rva, T& t, uint64_t size=0)
    {
        char* addrRet = addr(rva);
        if (addrRet)
        {
            memcpy(addrRet, &t, size>0 ? size : sizeof(T)); // unsafe, may be out of memory
        }
    }

private:
    friend class PEParser;
    class PEParser* m_pData;

    std::string                                     m_filePath;

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

class VectorDataOpr
{
public:
    VectorDataOpr(std::vector<unsigned char>& bytesArr)
        : m_uCurrentPosition(0)
        , m_bytesArr(bytesArr)
    {
    }


    VectorDataOpr& modifyVectorByteData(char* t, int len, unsigned int uOffset = -1)
    {
        if (-1 == uOffset)
        {
            uOffset = m_uCurrentPosition + 1;
        }
        if (uOffset + len >= m_bytesArr.size())
        {
            // out of memory
            assert(0);
            return *this;
        }

        for (unsigned int i = 0; i < len; i++)
        {
            m_bytesArr[uOffset + i] = *((unsigned char*)(t + i));
        }
        m_bytesArr[uOffset + len] = 0;

        m_uCurrentPosition = uOffset + len - 1;

        return *this;
    }

    template <typename T>
    VectorDataOpr& modifyVectorByteData(T& t, unsigned int uOffset = -1)
    {
        if (-1 == uOffset)
        {
            uOffset = m_uCurrentPosition + 1;
        }
        if (uOffset + sizeof(T) >= m_bytesArr.size())
        {
            // out of memory
            assert(0);
            return *this;
        }

        for (unsigned int i = 0; i < sizeof(T); i++)
        {
            m_bytesArr[uOffset + i] = *((unsigned char*)((char*)&t + i));
        }

        m_uCurrentPosition = uOffset + sizeof(T) - 1;

        return *this;
    }

    int CurrentPosition() const
    {
        return m_uCurrentPosition;
    }

private:
    unsigned int                m_uCurrentPosition;
    std::vector<unsigned char>& m_bytesArr;
};

}


#endif // PEPARSERPRIVATE_H
