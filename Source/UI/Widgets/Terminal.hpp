#pragma once

#include <QObject>
#include <QStringList>

// Terminal
// --------
// Stateless command builder. Constructs the correct sudo/doas shell invocation
// and emits it as a string for TerminalWidget::execute() to send into the PTY.
// Keeps command-building logic out of UI code.
class Terminal : public QObject
{
    Q_OBJECT

public:
    explicit Terminal(QObject *parent = nullptr);

    // Returns "doas" if /usr/bin/doas + /etc/doas.conf exist, else "sudo"
    static QString sudoProgram();

    // Run a single command string with privilege escalation
    void runWithSudo(const QString &command);

    // Run a list of commands joined as a shell -c "..." invocation with sudo
    void runWithSudo(const QStringList &commands);

    // Run a list of commands as the current user (e.g. AUR clone, makepkg)
    void runAsUser(const QStringList &commands);

    signals:
        // Connect this to TerminalWidget::execute()
        void commandReady(const QString &command);
};