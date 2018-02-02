#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>

void LogMessage(String asMsg = "", u32 aiLevel = 3, String asFile = "", int aiLine = -1);

int main(int argc, char *argv[])
{
    // Parse command-line args.
    QString lOpenFilePath = "";

#if 0
    for (int iIdx = 0; iIdx < argc; ++iIdx)
    {
        if (!strcmp("-f", argv[iIdx]) || !strcmp("--file", argv[iIdx]))
        {
            lOpenFilePath = argv[iIdx+1];
        }
    }
#endif

    if (1 < argc)
    {
        lOpenFilePath = argv[(argc-1)];
    }

    QApplication a(argc, argv);
    CMainWindow w;
    w.show();

    if (QFile(lOpenFilePath).exists() && !lOpenFilePath.isEmpty())
    {
        w.OpenImage(lOpenFilePath);
    }

    return a.exec();
}

/*
 * 0 = Fatal
 * 1 = Critical/Error
 * 2 = Warning
 * 3 = Information
 * 4 = Debug
 * 5 = Trace
 */
void LogMessage(String asMsg, u32 aiLevel, String asFile, int aiLine)
{
    switch(aiLevel)
    {
        case 0: // FATAL ERROR
        /*qFatal()*/qCritical().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] FATAL ERROR: "<< QString::fromStdString(asMsg)<< " <"<< QString::fromStdString(asFile)<< ":"<< aiLine<< ">";
        {
            break;
        }
        case 1: // ERROR
        {
            qCritical().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] ERROR: "<< QString::fromStdString(asMsg)<< " <"<< QString::fromStdString(asFile)<< ":"<< aiLine<< ">";
            break;
        }
        case 2: // WARNING
        {
            qWarning().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] WARN: "<< QString::fromStdString(asMsg)<< " <"<< QString::fromStdString(asFile)<< ":"<< aiLine<< ">";
            break;
        }
        case 3: // INFO
        default:
        {
            qInfo().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] INFO: "<< QString::fromStdString(asMsg);
            break;
        }
        case 4: // DEBUG
        {
            qDebug().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] DEBUG: "<< QString::fromStdString(asMsg)<< " <"<< QString::fromStdString(asFile)<< ":"<< aiLine<< ">";
            break;
        }
        case 5: // TRACE
        {
            qDebug().nospace().noquote() << "["<< QDateTime::currentSecsSinceEpoch()<< "] TRACE: "<< QString::fromStdString(asMsg)<< " <"<< QString::fromStdString(asFile)<< ":"<< aiLine<< ">";
            break;
        }
    }
}
