#include "Terminal.hpp"

#include <QFile>

Terminal::Terminal(QObject *parent)
    : QObject(parent)
{
}

QString Terminal::sudoProgram()
{
    if (QFile::exists("/usr/bin/doas") && QFile::exists("/etc/doas.conf"))
        return "doas";
    return "sudo";
}

void Terminal::runWithSudo(const QString &command)
{
    const QString cmd = sudoProgram() + " /bin/bash -c \"" + command + '"';
    emit commandReady(cmd);
}

void Terminal::runWithSudo(const QStringList &commands)
{
    // Join all commands into a single bash -c "..." invocation
    QString joined;
    for (const QString &line : commands)
        joined += line + '\n';

    const QString cmd = sudoProgram() + " /bin/bash -c \"" + joined + '"';
    emit commandReady(cmd);
}

void Terminal::runAsUser(const QStringList &commands)
{
    QString joined;
    for (const QString &line : commands)
        joined += line + '\n';

    const QString cmd = "/bin/bash -c \"" + joined + '"';
    emit commandReady(cmd);
}