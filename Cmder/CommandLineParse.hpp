#ifndef COMMANDLINEPARSE_H
#define COMMANDLINEPARSE_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <iostream>
#include <set>

#include "ICmder.h"


namespace Cmder
{

static bool MatchCMD(QString cmdSrc, QStringList cmdList)
{
    foreach (QString cmdItem, cmdList)
    {
        if (0 == cmdItem.compare(cmdSrc, Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

class CommandLineParse
{
private:
    enum class CMD_Mode
    {
        Once = 0,
        Interactive
    };

public:
    static CommandLineParse* GetInstance()
    {
        static CommandLineParse* pCommandLineParse = nullptr;
        if (!pCommandLineParse)
        {
            pCommandLineParse = new CommandLineParse;
        }
        return pCommandLineParse;
    }

    CommandLineParse& InitMainEntryPointParams(int argc, char *argv[])
    {
        m_cmdList = ConvertStringListFormat(argc, argv);
        m_cmdList.removeFirst(); // remove the module name
        m_CMD_Mode = (m_cmdList.isEmpty() ? CMD_Mode::Interactive : CMD_Mode::Once);

        return *this;
    }

    void exec()
    {
        while (true)
        {
            const int cmdCount = m_cmdList.count();

            // params mode
            if (cmdCount < 1)
            {
                std::cout << "Enter interactive mdoe." << std::endl;
                goto CMD_MODE;
            }


            bool processedFlag = false;
            for (auto iter = m_CmderSet.begin(); iter != m_CmderSet.end(); iter++)
            {
                ICmder* pCmder = *iter;
                ICmder::ErrorCode iRet = pCmder->ChangeEvent(m_cmdList);
                if (ICmder::ErrorCode::SUCCESS == iRet && !processedFlag)
                {
                    processedFlag = true;
                }
            }

            if (!processedFlag)
            {
                std::cout << "Nothing to parse" << std::endl;
                goto CMD_MODE;
            }

            if (CMD_Mode::Once == m_CMD_Mode)
            {
                break;
            }

            // command mode
CMD_MODE:
            m_CMD_Mode = CMD_Mode::Interactive;
            std::cout<<">>";
            char input_cmd[4096]{0};
            std::cin.getline(input_cmd, sizeof(input_cmd)/sizeof(char));
            QString qStrCmd = input_cmd;
            if (MatchCMD(qStrCmd, QStringList()<<"q"<<"quit"))
            {
                break;
            }

            m_cmdList = MatchSubString("(\".*?\")||(\\S+)", qStrCmd, 2);
        }
    }

    CommandLineParse& AddCmder(ICmder* pCmder)
    {
        m_CmderSet.insert(pCmder);
        return *this;
    }

    CommandLineParse& DeleteCmder(ICmder* pCmder)
    {
        for (auto iter = m_CmderSet.begin(); iter != m_CmderSet.end(); iter++)
        {
            if (*iter == pCmder)
            {
                m_CmderSet.erase(iter);
                break;
            }
        }
        return *this;
    }

private:
    CommandLineParse() = default;

    QStringList ConvertStringListFormat(int argc, char *argv[]) const
    {
        QStringList stringList;
        for (int i=0; i<argc; i++)
        {
            stringList.push_back(argv[i]);
        }
        return stringList;
    }

    QStringList MatchSubString(QString regexp, QString srcString, int nGroupCount) const
    {
        QStringList matchedList;
        QRegularExpression re(regexp);
        QRegularExpressionMatchIterator iter = re.globalMatch(srcString);
        while (iter.hasNext())
        {
            QRegularExpressionMatch match = iter.next();
            for (int i = 0; i < nGroupCount; i++)
            {
                QString subStr = match.captured(i + 1);
                if (!subStr.isEmpty())
                {
                    matchedList << subStr;
                }
            }
        }
        return matchedList;
    }

private:
    CMD_Mode          m_CMD_Mode;
    QStringList       m_cmdList;
    std::set<ICmder*> m_CmderSet;
};


template<typename T>
class CmderHelper
{
public:
    CmderHelper()
    {
        m_pCmder = new T;
        CommandLineParse::GetInstance()->AddCmder(m_pCmder);
    }
    ~CmderHelper()
    {
        if (!m_pCmder)
        {
            CommandLineParse::GetInstance()->DeleteCmder(m_pCmder);
        }
    }

private:
    ICmder* m_pCmder;
};

#define REGIST_CMDER_OBJ(_CLASS_) Cmder::CmderHelper<_CLASS_> the##_CLASS_;


}

#endif // COMMANDLINEPARSE_H
