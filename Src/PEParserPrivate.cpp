#include "PEParserPrivate.h"
#include "PEParser.h"

using namespace PEInjector;

PEParserPrivate::PEParserPrivate(PEParser *pPEParser)
    : m_pData(pPEParser)
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
