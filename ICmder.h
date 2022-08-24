#ifndef ICMDER_H
#define ICMDER_H

#include <string>
#include <list>

class ICmder
{
public:
    template<typename T>
    ICmder(T* pThis);
    virtual ~ICmder();
    virtual void ChangeEvent(ICmder* cmderOwner, std::list<std::string> cmderList) = 0;
};

#endif // ICMDER_H
