#ifndef PESTRUCT_H
#define PESTRUCT_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace PEStruct
{

constexpr const char* MagicDOSSignature = "MZ";

struct DOSHeader
{
    std::uint8_t  magic[2];
    std::uint16_t lastSize;
    std::uint16_t numberBlocks;
    std::uint16_t numberRelocations;
    std::uint16_t hdrSize;
    std::uint16_t minAlloc;
    std::uint16_t maxAlloc;
    std::uint16_t ss;
    std::uint16_t sp;
    std::uint16_t checksum;
    std::uint16_t ip;
    std::uint16_t cs;
    std::uint16_t relocPos;
    std::uint16_t numberOverlay;
    std::uint16_t reserved1[4];
    std::uint16_t oem_id;
    std::uint16_t oem_info;
    std::uint16_t reserved2[10];
    std::uint32_t pointerToCoffHeader;
};

constexpr const std::uint32_t MagicPESignature = 0x00004550;  // PE00
constexpr const std::uint16_t x86Signature = 0x014c;

struct COFFHeader
{
    std::uint32_t magic;
    std::uint16_t machine;
    std::uint16_t numberSections;
    std::uint16_t timeDateStamp;
    std::uint32_t pointerToSymbolTable;
    std::uint32_t numberOfSymbols;
    std::uint16_t sizeOfOptionalHeader;
    std::uint16_t characteristics;
};

struct DataDirectory
{
    std::uint32_t virtualAddress;
    std::uint32_t size;
};

constexpr const int MaxDirectoryEntryCount = 16;
constexpr const int MaxSectionCount = 64;

struct PEOptHeader
{
    // Standard fields
    std::uint16_t signature;
    std::uint8_t  majorLinkerVersion;
    std::uint8_t  minorLinkerVersion;
    std::uint32_t sizeOfCode;
    std::uint32_t sizeOfInitializedData;
    std::uint32_t sizeOfUninitializedData;
    std::uint32_t addressOfEntryPoint;
    std::uint32_t baseOfCode;
    std::uint32_t baseOfData;
    // NT additional fields
    std::uint32_t imageBase;
    std::uint32_t sectionAlignment;
    std::uint32_t fileAlignment;
    std::uint16_t majorOSVersion;
    std::uint16_t minorOSVersion;
    std::uint16_t majorImageVersion;
    std::uint16_t minorImageVersion;
    std::uint16_t majorSubsystemVersion;
    std::uint16_t minorSubsystemVersion;
    std::uint32_t win32VersionValue;
    std::uint32_t sizeOfImage;
    std::uint32_t sizeOfHeaders;
    std::uint32_t checksum;
    std::uint16_t subsystem;
    std::uint16_t dLLCharacteristics;
    std::uint32_t sizeOfStackReserve;
    std::uint32_t sizeOfStackCommit;
    std::uint32_t sizeOfHeapReserve;
    std::uint32_t sizeOfHeapCommit;
    std::uint32_t loaderFlags;
    std::uint32_t numberOfRvaAndSizes;
    DataDirectory dataDirectory[MaxDirectoryEntryCount];
};

struct SectionHeader
{
    char name[8];

    union
    {
        std::uint32_t   physicalAddress;
        std::uint32_t   virtualSize;
    } Misc;

    std::uint32_t   virtualAddress;
    std::uint32_t   sizeOfRawData;
    std::uint32_t   pointerToRawData;
    std::uint32_t   pointerToRelocations;
    std::uint32_t   pointerToLinenumbers;
    std::uint16_t   numberOfRelocations;
    std::uint16_t   numberOfLinenumbers;
    std::uint32_t   characteristics;
};

constexpr const int DirectoryEntryImport = 1;

struct ImageImportDescriptor
{
    std::uint32_t   originalFirstThunk;
    std::uint32_t   timeDateStamp;
    std::uint32_t   forwarderChain;
    std::uint32_t   rvaName;
    std::uint32_t   firstThunk;
};

#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)

struct Thunk
{
    union
    {
        std::uint32_t ForwarderString;
        std::uint32_t Function;
        std::uint32_t Ordinal;
        std::uint32_t AddressOfData;
    };
};

// An import Entry can either be a hint + functionName or a functionOrdinal
struct ImportEntry
{
    std::uint16_t hint;
    std::string functionName;

    std::uint32_t functionOrdinal;
};

struct DllImport
{
    std::string name;
    std::vector<ImportEntry> entries_;
};

constexpr const int DirectoryEntryReloc = 5;
struct ImageBaseReloction
{
    std::uint32_t   virtualAddress;
    std::uint32_t   sizeOfBlock;
};
constexpr const int ImageSizeOfBaseReloction = sizeof(ImageBaseReloction);
struct ImageRelocItem
{
    uint32_t rva;
    uint8_t  type;
    uint32_t foa;
};

}

#endif // PESTRUCT_H
