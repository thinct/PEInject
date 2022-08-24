#include "ICmder.h"
#include "CommandLineParse.h"

template<typename T>
ICmder::ICmder(T* pThis)
{
    CommandLineParse::GetInstance()->AddCmder(pThis);
}

template<typename T>
ICmder::~ICmder()
{
    // CommandLineParse::GetInstance()->DeleteCmder(pThis);
}
