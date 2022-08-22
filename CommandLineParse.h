#ifndef COMMANDLINEPARSE_H
#define COMMANDLINEPARSE_H

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
                    LoadImportTable(m_cmdList[1].toStdString());
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
            std::string input_cmd;
            std::cin>>input_cmd;
            QString qStrCmd = input_cmd.data();
            if (MatchCMD(qStrCmd, QStringList()<<"q"<<"quit"))
            {
                break;
            }

            m_cmdList = qStrCmd.split(QRegExp(" +"));
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

private:
    CMD_Mode    m_CMD_Mode;
    QStringList m_cmdList;
};

#endif // COMMANDLINEPARSE_H
