#include "PEParser.h"
#include <cstdint>
#include <cstring>
#include <map>

#include "PEStruct.h"
#include "PEParserPrivate.h"
#include "Global/BinaryFile.hpp"
using namespace PEStruct;
using namespace FileSystem;
using namespace PEInjector;

PEParser::PEParser(const std::string& pePath)
    : m_pData(new PEParserPrivate(this, pePath))
{
}

PEParser& PEParser::ReadHeadersAndSections()
{
    BinaryFile f(m_pData->m_filePath, std::fstream::in);

    std::cout << "Reading DOS header" << std::endl;
    f.read(m_pData->m_dosHeader);

    if (!m_pData->checkDOSHeader())
    {
        std::cout << "Invalid DOS header, are you sure '" << m_pData->m_filePath << "' is a PE?" << std::endl;
        throw std::runtime_error("Invalid DOS header");
    }

    std::cout << "Reading DOS stub" << std::endl;
    m_pData->m_dosStub = f.readBuffer(m_pData->m_dosHeader.pointerToCoffHeader - sizeof(DOSHeader));

    std::cout << "Reading Coff Header" << std::endl;
    f.read(m_pData->m_coffHeader);

    if (!m_pData->checkPESignature())
    {
        std::cout << "Invalid Coff header, are you sure '" << m_pData->m_filePath << "' is a PE?" << std::endl;
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

    std::size_t paddingSize = m_pData->m_sectionHeaders[0].pointerToRawData - f.tell();// may be space to add new section.
    m_pData->m_padding = f.readBuffer(paddingSize);

    for (const auto& sectionHeader : m_pData->m_sectionHeaders)
    {
        f.seek(sectionHeader.pointerToRawData);
        m_pData->m_sections.push_back(f.readBuffer(sectionHeader.sizeOfRawData));
    }

    f.close();

    return *this;
}

PEParser &PEParser::LoadImportTable()
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
        printf("OFTs:%X(->%X)\tIAT:%X(->%X)\n", imageImportDescriptor.originalFirstThunk, RvaToFileOffset(imageImportDescriptor.originalFirstThunk)
            , imageImportDescriptor.firstThunk, RvaToFileOffset(imageImportDescriptor.firstThunk));
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

            import.entries_.push_back(entry);
            ++thunkIndex;
        }

        m_pData->m_importedDlls.push_back(import);

        ++descIndex;
        std::cout << std::endl;
    }

    std::cout << std::endl;

    return *this;
}

PEParser &PEParser::LoadReloctionTable()
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

    return *this;
}

PEParser& PEInjector::PEParser::RebuildImportTable(std::string tagName, std::uint32_t fileOffset)
{
    std::vector<unsigned char>& sectionData = m_pData->section(tagName);
    if (sectionData.size() <= fileOffset)
    {
        return *this;
    }

    struct FirstThunkRVAItem
    {
        std::uint32_t OFTs;
        std::uint32_t IATs;
    };
    std::map<std::string, std::vector < FirstThunkRVAItem>> mapModuleFirstThunkItems;

    SectionHeader& sectionHeader = m_pData->sectionHeader(tagName);
    std::vector<DllImport> importedDlls = m_pData->m_importedDlls;
    for (unsigned int descIndex = 0; descIndex < importedDlls.size(); descIndex++)
    {
        std::vector < FirstThunkRVAItem>& vFirstThunkRVAItems = mapModuleFirstThunkItems[importedDlls[descIndex].name];
        vFirstThunkRVAItems.clear();
        for (unsigned int thunkIndex=0; thunkIndex<importedDlls[descIndex].entries_.size(); thunkIndex++)
        {
            std::uint32_t hintVirtualAddr = sectionHeader.virtualAddress + fileOffset;
            std::uint32_t funcNameVirtualAddr = hintVirtualAddr + sizeof(std::uint16_t);
            vFirstThunkRVAItems.push_back(FirstThunkRVAItem{ hintVirtualAddr , funcNameVirtualAddr });

            PEStruct::ImportEntry entry = importedDlls[descIndex].entries_[thunkIndex];
            VectorDataOpr& vectorDataOpr = VectorDataOpr(sectionData)
                .modifyVectorByteData(entry.hint, fileOffset)
                .modifyVectorByteData((char*)entry.functionName.data(), entry.functionName.length()+1);
            fileOffset = vectorDataOpr.CurrentPosition()+1;
        }
    }


    std::vector< ImageImportDescriptor> vImageImportDescriptor;
    for (unsigned int descIndex = 0; descIndex < importedDlls.size(); descIndex++)
    {
        std::uint32_t moduleNameVirtualAddr = sectionHeader.virtualAddress + fileOffset;

        VectorDataOpr& vectorDataOpr = VectorDataOpr(sectionData);
        vectorDataOpr.modifyVectorByteData((char*)importedDlls[descIndex].name.data(), importedDlls[descIndex].name.length()+1, fileOffset);
        fileOffset = vectorDataOpr.CurrentPosition()+1;

        ImageImportDescriptor imageImportDescriptor;
        imageImportDescriptor.rvaName = moduleNameVirtualAddr;
        imageImportDescriptor.forwarderChain = 0;
        imageImportDescriptor.timeDateStamp = 0;

        const std::vector < FirstThunkRVAItem>& vFirstThunkRVAItems = mapModuleFirstThunkItems[importedDlls[descIndex].name];
        imageImportDescriptor.originalFirstThunk = sectionHeader.virtualAddress + fileOffset;
        for (unsigned int index = 0; index < vFirstThunkRVAItems.size(); index++)
        {
            vectorDataOpr.modifyVectorByteData(vFirstThunkRVAItems[index].OFTs);
        }
        uint32_t endByZero = 0;
        vectorDataOpr.modifyVectorByteData(endByZero);
        fileOffset = vectorDataOpr.CurrentPosition();

        imageImportDescriptor.firstThunk = sectionHeader.virtualAddress + fileOffset;
        for (unsigned int index = 0; index < vFirstThunkRVAItems.size(); index++)
        {
            vectorDataOpr.modifyVectorByteData(vFirstThunkRVAItems[index].IATs);
        }
        vectorDataOpr.modifyVectorByteData(endByZero);
        fileOffset = vectorDataOpr.CurrentPosition();

        vImageImportDescriptor.push_back(imageImportDescriptor);
    }

    fileOffset++;

    m_pData->m_peOptHeader.dataDirectory[DirectoryEntryImport].virtualAddress = sectionHeader.virtualAddress + fileOffset;
    for (unsigned int i = 0; i < vImageImportDescriptor.size(); i++)
    {
        VectorDataOpr& vectorDataOpr = VectorDataOpr(sectionData);
        vectorDataOpr.modifyVectorByteData(vImageImportDescriptor[i].originalFirstThunk, fileOffset);
        vectorDataOpr.modifyVectorByteData(vImageImportDescriptor[i].timeDateStamp);
        vectorDataOpr.modifyVectorByteData(vImageImportDescriptor[i].forwarderChain);
        vectorDataOpr.modifyVectorByteData(vImageImportDescriptor[i].rvaName);
        vectorDataOpr.modifyVectorByteData(vImageImportDescriptor[i].firstThunk);
        fileOffset = vectorDataOpr.CurrentPosition()+1;
    }

    return *this;
}

PEParser& PEInjector::PEParser::RebuildRelocTable(std::string tagName, std::uint32_t fileOffset)
{
    // TODO: 在此处插入 return 语句
    return *this;
}

PEParser& PEInjector::PEParser::InjectDll(std::string moduleName, std::string funcName)
{
    // TODO: 在此处插入 return 语句
    return *this;
}

PEParser& PEInjector::PEParser::InjectCode(std::uint32_t rvaHook, std::uint8_t lenCode, std::string tagName, std::uint32_t fileOffset)
{
    // TODO: 在此处插入 return 语句
    return *this;
}

PEParser &PEParser::AppendNewSection(std::string tagName, uint32_t size, uint32_t characteristics)
{
    std::cout << "Append new section:\t" << tagName << std::endl;

    uint32_t space_nt_dos = m_pData->m_peOptHeader.sizeOfHeaders - sizeof(SectionHeader)*m_pData->m_coffHeader.numberSections
            - sizeof(PEOptHeader)- sizeof(COFFHeader) - m_pData->m_dosHeader.pointerToCoffHeader;

    bool isNeedUpHeader = false;
    if (space_nt_dos<sizeof(SectionHeader)*2)
    {
        printf("The space is not enough!\n");
        isNeedUpHeader = true;
    }

    if (isNeedUpHeader)
    {
        uint32_t dosStubSize = m_pData->m_dosHeader.pointerToCoffHeader - sizeof(DOSHeader);
        if (space_nt_dos+dosStubSize<sizeof(SectionHeader)*2)
        {
            printf("The space(contain dos stub part) is not enough!\n");
            return *this;
        }
        return *this; // Do not process it now!
    }

    SectionHeader oldLastSectionHeader = m_pData->m_sectionHeaders[m_pData->m_sectionHeaders.size()-1];
    SectionHeader newSectionHeader = oldLastSectionHeader;

    uint32_t expand = m_pData->align(size, m_pData->m_peOptHeader.fileAlignment);
    m_pData->m_peOptHeader.sizeOfImage += expand;
    memcpy(newSectionHeader.name, tagName.data(), 8);
    newSectionHeader.Misc.virtualSize = expand;
    newSectionHeader.virtualAddress = m_pData->m_peOptHeader.sizeOfImage - expand;
    newSectionHeader.sizeOfRawData = expand;
    newSectionHeader.pointerToRawData = oldLastSectionHeader.pointerToRawData + oldLastSectionHeader.sizeOfRawData;
    newSectionHeader.characteristics = characteristics;

    m_pData->m_sectionHeaders.push_back(newSectionHeader);
    std::vector<unsigned char> newBuffer;
    newBuffer.resize(expand);
    m_pData->m_sections.push_back(newBuffer);

    m_pData->m_coffHeader.numberSections = m_pData->m_sectionHeaders.size();

    return *this;
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

void PEParser::SaveAs(const std::string &pePath)
{
    BinaryFile f(pePath, std::fstream::out);

    std::cout << "Writing DOS header" << std::endl;
    f.write(m_pData->m_dosHeader);

    std::cout << "Writing DOS stub" << std::endl;
    f.writeBuffer(m_pData->m_dosStub);

    std::cout << "Writing Coff Header" << std::endl;
    f.write(m_pData->m_coffHeader);

    if (!m_pData->checkPESignature())
    {
        std::cout << "Invalid Coff header, are you sure '" << m_pData->m_filePath << "' is a PE?" << std::endl;
        throw std::runtime_error("Invalid Coff header");
    }

    std::cout << "Writing PE Opt Header" << std::endl;
    f.write(m_pData->m_peOptHeader);

    std::cout << "Writing sections headers" << std::endl;
    std::cout << "Number of sections: " << m_pData->m_coffHeader.numberSections << std::endl;

    for (int i = 0; i < m_pData->m_coffHeader.numberSections; ++i)
    {
        f.write<SectionHeader>(m_pData->m_sectionHeaders[i]);
    }

    std::size_t paddingSize = m_pData->m_sectionHeaders[0].pointerToRawData - f.tell();// may be space to add new section.
    std::cout<<"padding size: "<<paddingSize;
    std::vector<unsigned char> paddingBuffer;
    paddingBuffer.resize(paddingSize);
    f.writeBuffer(paddingBuffer);

    for (int i=0; i<m_pData->m_sections.size(); i++)
    {
        f.writeBuffer(m_pData->m_sections[i]);
    }

    f.close();

}
