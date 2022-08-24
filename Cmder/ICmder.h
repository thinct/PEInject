#ifndef ICMDER_H
#define ICMDER_H

#include <QStringList>
namespace Cmder
{

class ICmder
{
public:
    enum class ErrorCode
    {
        FAILED = -1,
        SUCCESS = 0
    };

public:
    virtual ErrorCode ChangeEvent(QStringList cmderList) = 0;
    virtual bool      CheckVaild(QStringList cmderList) = 0;
};

}
#endif // ICMDER_H
