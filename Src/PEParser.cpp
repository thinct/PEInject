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
    ReadHeadersAndSections(pePath);
}

void PEParser::ReadHeadersAndSections(const std::string& pePath)
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

void PEParser::LoadImportTable()
{
    int descIndex = 0;

    std::cout << "Reading import table:" << std::endl;

    // Read contiguous ImageImportDescriptors until one has null fields.
    while (true)
    {
        ImageImportDescriptor imageImportDescriptor;
        m_pData->readSection(m_pData->m_peOptHeader.dataDirectory[DirectoryEntryImport].virtualAddress
                             + descIndex * sizeof(ImageImportDescriptor)
                             , imageImportDescriptor);

        if (0 == imageImportDescriptor.rvaName)
        {
            break; // Checking if the rvaName empty is enough.
        }

        DllImport import;
        import.name = std::string(m_pData->addr(imageImportDescriptor.rvaName));

        std::cout << import.name << std::endl;
        std::cout << "======================" << std::endl;

        int thunkIndex = 0;

        while (true)
        {
            Thunk thunk;
            m_pData->readSection(imageImportDescriptor.originalFirstThunk + thunkIndex * sizeof(Thunk), thunk);

            if (0 == thunk.AddressOfData)
            {
                break;
            }

            ImportEntry entry;
            if ((thunk.Ordinal & IMAGE_ORDINAL_FLAG32) == IMAGE_ORDINAL_FLAG32)
            {
                entry.functionOrdinal = thunk.Ordinal;
                std::cout << "- Ordinal: " << thunk.Ordinal << std::endl;
            }
            else
            {
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

void PEParser::LoadReloctionTable()
{
    std::cout << "Reading import table:" << std::endl;
    int descIndex = 0;
    int blockOffset = 0;
    while (true)
    {
        std::uint32_t rvaRelocTable = m_pData->m_peOptHeader.dataDirectory[DirectoryEntryReloc].virtualAddress + blockOffset;
        ImageBaseReloction imageBaseReloction;
        m_pData->readSection(rvaRelocTable, imageBaseReloction);

        // Read contiguous ImageImportDescriptors until one has null fields.
        if (!imageBaseReloction.sizeOfBlock && !imageBaseReloction.virtualAddress)
        {
            break; // Checking if the rvaName empty is enough.
        }
        blockOffset += imageBaseReloction.sizeOfBlock;
        ++descIndex;

        std::cout<<"The"<<" "<<descIndex<<" "<<"th block."<<std::endl;
        std::cout<<"***********************************************************************************************************"<<std::endl;

        printf("imageBaseReloction.virtualAddress :      %X\n",   imageBaseReloction.virtualAddress);
        printf("imageBaseReloction.sizeOfBlock    :      %X\n",   imageBaseReloction.sizeOfBlock);
        printf("NumberOfRelocAddress              :      %d\n\n", (imageBaseReloction.sizeOfBlock - 8)/2);

        printf("the_RVA_of_RELOC(X)          type(3or!3)          FOA(X)          \n");
        int NumberOfRelocAddress = (imageBaseReloction.sizeOfBlock - 8) / 2;
        for(int i = 0; i < NumberOfRelocAddress; i++)
        {
            std::uint16_t specificItem;
            m_pData->readSection(rvaRelocTable + ImageSizeOfBaseReloction + i*sizeof(specificItem), specificItem);
			m_pData->m_relocTables[imageBaseReloction.virtualAddress].push_back(
                ImageRelocItem{
					(specificItem & 0x0fff) + imageBaseReloction.virtualAddress
					, (uint8_t)(specificItem >> 12)
				    , RvaToFileOffset((specificItem & 0x0fff) + imageBaseReloction.virtualAddress)
                }
            );

            printf("%-30X", (specificItem & 0x0fff) + imageBaseReloction.virtualAddress);
            printf("%-21X", specificItem >> 12);
            printf("%-16X", RvaToFileOffset((specificItem & 0x0fff) + imageBaseReloction.virtualAddress));
            printf("\n");
        }
    }

    std::cout << std::endl;
}

uint32_t PEParser::RvaToFileOffset(uint32_t dwRva)
{
    if(dwRva <= m_pData->m_peOptHeader.sizeOfHeaders)
    {
        return dwRva;
    }
    for(uint32_t i=0;i<m_pData->m_coffHeader.numberSections;i++)
    {
        if(dwRva >= m_pData->m_sectionHeaders[i].virtualAddress && dwRva < (m_pData->m_sectionHeaders[i].virtualAddress + m_pData->m_sectionHeaders[i].Misc.virtualSize))
        {
            return m_pData->m_sectionHeaders[i].pointerToRawData + dwRva - m_pData->m_sectionHeaders[i].virtualAddress;
        }
    }

    return -1;
}
