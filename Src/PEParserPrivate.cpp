#include "PEParserPrivate.h"
#include <assert.h>
#include "PEParser.h"

using namespace PEInjector;

PEParserPrivate::PEParserPrivate(PEParser *pPEParser, const std::string filePath)
    : m_pData(pPEParser)
    , m_filePath(filePath)
{

}

char *PEParserPrivate::addr(uint32_t rva)
{
    for (int i = 0; i < m_sectionHeaders.size(); ++i)
    {
        if (rva >= m_sectionHeaders[i].virtualAddress &&
                rva < m_sectionHeaders[i].virtualAddress + m_sectionHeaders[i].Misc.virtualSize)
        {

            return reinterpret_cast<char*>(m_sections[i].data()
                                           + rva
                                           - m_sectionHeaders[i].virtualAddress);
        }
    }

    return nullptr;
}

bool PEParserPrivate::checkDOSHeader()
{
    std::cout << "Checking DOS header" << std::endl;

    if (std::memcmp(MagicDOSSignature, m_dosHeader.magic, sizeof(m_dosHeader.magic)) != 0)
    {
        std::cout << "Invalid magic DOS number" << std::endl;

        return false;
    }

    return true;
}

bool PEParserPrivate::checkPESignature()
{
    std::cout << "Checking Coff header" << std::endl;

    if (m_coffHeader.magic != MagicPESignature)
    {
        std::cout << "Invalid magic PE number" << std::endl;

        return false;
    }

    if (m_coffHeader.machine != x86Signature)
    {
        std::cout << "Invalid machine signature" << std::endl;

        return false;
    }

    if (m_coffHeader.sizeOfOptionalHeader < sizeof(PEOptHeader))
    {
        std::cout << "Optional header too small" << std::endl;

        return false;
    }

    if (m_coffHeader.numberSections > MaxSectionCount)
    {
        std::cout << "The number of sections should be under " << MaxSectionCount << std::endl;

        return false;
    }

    return true;
}

uint32_t PEParserPrivate::align(uint32_t ad, uint32_t alignment)
{
    return (ad%alignment==0?ad:(ad/alignment+1)*alignment);
}

std::vector<unsigned char>& PEParserPrivate::section(std::string tagName, bool* ok)
{
    if (ok)
    {
        *ok = false;
    }

    int indexFound = -1;
    for (unsigned int i = 0; i < m_sectionHeaders.size(); i++)
    {
        if (m_sectionHeaders[i].name == tagName)
        {
            indexFound = i;
            break;
        }
    }

    if (-1 == indexFound)
    {
        return std::vector<unsigned char>();
    }

    assert(m_sectionHeaders.size() == m_sections.size());
    if (ok)
    {
        *ok = true;
    }
    return m_sections[indexFound];
}

SectionHeader& PEInjector::PEParserPrivate::sectionHeader(std::string tagName, bool* ok)
{
    if (ok)
    {
        *ok = false;
    }

    int indexFound = -1;
    unsigned int i = 0;
    for (; i < m_sectionHeaders.size(); i++)
    {
        if (m_sectionHeaders[i].name == tagName)
        {
            indexFound = i;
            break;
        }
    }

    if (-1 == indexFound)
    {
        return SectionHeader();
    }
    if (ok)
    {
        *ok = true;
    }
    assert(m_sectionHeaders.size() == m_sections.size());
    return m_sectionHeaders[i];
}
