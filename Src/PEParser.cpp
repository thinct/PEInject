#include "PEParser.h"
#include <cstdint>
#include <cstring>

#include "PEStruct.h"
#include "PEParserPrivate.h"
#include "Global/BinaryFile.hpp"
using namespace PEStruct;
using namespace FileSystem;
using namespace PEInjector;

PEParser::PEParser(const std::string& pePath)
    : m_pData(new PEParserPrivate(this))
{
    readHeadersAndSections(pePath);
}

void PEParser::readHeadersAndSections(const std::string& pePath)
{
    BinaryFile f(pePath, std::fstream::in);

    std::cout << "Reading DOS header" << std::endl;
    f.read(m_pData->m_dosHeader);

    if (!m_pData->checkDOSHeader())
    {
        std::cout << "Invalid DOS header, are you sure '" << pePath << "' is a PE?" << std::endl;
        throw std::runtime_error("Invalid DOS header");
    }

    std::cout << "Reading DOS stub" << std::endl;
    m_pData->m_dosStub = f.readBuffer(m_pData->m_dosHeader.pointerToCoffHeader - sizeof(DOSHeader));

    std::cout << "Reading Coff Header" << std::endl;
    f.read(m_pData->m_coffHeader);

    if (!m_pData->checkPESignature())
    {
        std::cout << "Invalid Coff header, are you sure '" << pePath << "' is a PE?" << std::endl;
        throw std::runtime_error("Invalid Coff header");
    }

    std::cout << "Reading PE Opt Header" << std::endl;
    f.read(m_pData->m_peOptHeader);

    std::cout << "Reading sections headers" << std::endl;
    std::cout << "Number of sections: " << m_pData->m_coffHeader.numberSections << std::endl;

    for (int i = 0; i < m_pData->m_coffHeader.numberSections; ++i)
    {
        m_pData->m_sectionHeaders.push_back(f.read<SectionHeader>());
    }

    std::size_t paddingSize = m_pData->m_sectionHeaders[0].pointerToRawData - f.tell();
    m_pData->m_padding = f.readBuffer(paddingSize);

    for (const auto& sectionHeader : m_pData->m_sectionHeaders)
    {
        f.seek(sectionHeader.pointerToRawData);
        m_pData->m_sections.push_back(f.readBuffer(sectionHeader.sizeOfRawData));
    }

    f.close();
}

void PEParser::loadImportTable()
{
    int descIndex = 0;

    std::cout << "Reading import table:" << std::endl;

    // Read contiguous ImageImportDescriptors until one has null fields.
    while (true)
    {
        ImageImportDescriptor imageImportDescriptor;
        m_pData->readSection(m_pData->m_peOptHeader.dataDirectory[DirectoryEntryImport].virtualAddress + descIndex * sizeof(ImageImportDescriptor),
                    imageImportDescriptor);

        if (imageImportDescriptor.rvaName == 0)
            break; // Checking if the rvaName empty is enough.

        DllImport import;
        import.name = std::string(m_pData->addr(imageImportDescriptor.rvaName));

        std::cout << import.name << std::endl;
        std::cout << "======================" << std::endl;

        int thunkIndex = 0;

        while (true)
        {
            Thunk thunk;
            m_pData->readSection(imageImportDescriptor.originalFirstThunk + thunkIndex * sizeof(Thunk), thunk);

            if (thunk.AddressOfData == 0)
                break;

            ImportEntry entry;

            if ((thunk.Ordinal & IMAGE_ORDINAL_FLAG32) == IMAGE_ORDINAL_FLAG32)
            {
                entry.functionOrdinal = thunk.Ordinal;
                std::cout << "- Ordinal: " << thunk.Ordinal << std::endl;
            } else {
                m_pData->readSection(thunk.AddressOfData, entry.hint);
                entry.functionName = m_pData->addr(thunk.AddressOfData + 2);

                std::cout << "- Hint: " << entry.hint << std::endl;
                std::cout << "  Name: " << entry.functionName << std::endl;
            }

            ++thunkIndex;
        }

        m_pData->m_importedDlls.push_back(import);

        ++descIndex;
    }

    std::cout << std::endl;
}
