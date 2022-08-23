#ifndef COMMANDLINEPARSE_H
#define COMMANDLINEPARSE_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <iostream>
#include <QString>
#include <QDebug>

#include "Tester/FileTest.hpp"
#include "Src/PEParser.h"

using namespace PEInjector;


class CommandLineParse
{
private:
    enum class CMD_Mode
    {
        Once = 0,
        Interactive
    };

public:
    CommandLineParse(int argc, char *argv[])
    {
        m_cmdList = ConvertStringListFormat(argc, argv);
        m_cmdList.removeFirst(); // remove the module name
        m_CMD_Mode = (m_cmdList.isEmpty() ? CMD_Mode::Interactive : CMD_Mode::Once);
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

            if (1 == cmdCount)
            {
                if (MatchCMD(m_cmdList[0], QStringList()<<"detach"))
                {

                }
            }
            else if (2 == cmdCount)
            {
                if (MatchCMD(m_cmdList[0], QStringList()<<"int"))
                {
                    CustomINT(m_cmdList[0].toInt());
                }
                if (MatchCMD(m_cmdList[0]
                             , QStringList()<<"DisplayImportTable"<<"DIT"))
                {
                    LoadImportTable(m_cmdList[1].remove("\"").toStdString());
                }
            }
            else
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

private:
    QStringList ConvertStringListFormat(int argc, char *argv[]) const
    {
        QStringList stringList;
        for (int i=0; i<argc; i++)
        {
            stringList.push_back(argv[i]);
        }
        return stringList;
    }

    bool MatchCMD(QString cmdSrc, QStringList cmdList) const
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

    void CustomINT(int number)
    {

    }

    void LoadImportTable(std::string peFilePath)
    {
        PEParser parser(peFilePath);
        parser.loadImportTable();
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
    CMD_Mode    m_CMD_Mode;
    QStringList m_cmdList;
};

#endif // COMMANDLINEPARSE_H
