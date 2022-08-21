#ifndef FILE_TEST_HPP
#define FILE_TEST_HPP

#include <iostream>
#include <QDebug>

#include "Global/Common.hpp"
#include "Global/BinaryFile.hpp"
using namespace FileSystem;

void TestFile()
{
    const std::string binFilePath = "D:/test";

    BinaryFile file = BinaryFile(binFilePath, std::fstream::in|std::fstream::out);

    uchar buf[16]{0};
    file.read(buf);
    DisplayByteArray(buf, sizeof(buf)/sizeof(uchar));
    std::cout<<__LINE__<<std::endl;

    file.seek(0);

    std::cout<<__LINE__<<"\t"<< file.size() <<std::endl;


    std::vector<uchar> vBuffer = file.readBuffer(file.size());
    DisplayByteArray(vBuffer);
    std::cout<<__LINE__<<"\t"<<vBuffer.size()<<std::endl;


    file.seek(0, std::ios_base::end);


    uchar bufferAdd[] = {'G', 'O', 'O', 'D'};
    file.write(bufferAdd);

    file.flush();

    std::vector<uchar> vBufferAdd;
    vBufferAdd.push_back('H');
    vBufferAdd.push_back('H');
    vBufferAdd.push_back('H');
    file.writeBuffer(vBufferAdd);

    file.flush();
}

#endif // FILE_TEST_HPP
