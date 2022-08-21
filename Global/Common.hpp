#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <QtGlobal>
#include <iostream>
#include <vector>

void DisplayByteArray(uchar* pBuffer, int size)
{
    for (int i=0; i<size; i++)
    {
        std::cout<<std::hex << std::uppercase <<(int)pBuffer[i]<<" ";
    }
    std::cout<<std::endl;
    std::cout<<std::dec;
}

void DisplayByteArray(std::vector<uchar> vBuf)
{
    for (int i=0; i<vBuf.size(); i++)
    {
        std::cout<<std::hex << std::uppercase <<(int)vBuf[i]<<" ";
    }
    std::cout<<std::endl;
    std::cout<<std::dec;
}


#endif // GLOBAL_HPP
