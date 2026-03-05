#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QProcess>
#include <QStringList>

// KernelInstallDialog
// -------------------
// Modal "Installation Details" panel shown when the user clicks Install Selected.
// Runs `pacman -Sp --print-format "%n\t%v\t%r\t%d"` to resolve the full
// dependency set and displays it in a table before the user confirms.
//
// On Accepted → caller runs pkexec pacman -S <targets>.

class KernelInstallDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KernelInstallDialog(const QStringList &packageNames,
                                 QWidget *parent = nullptr);
    ~KernelInstallDialog() override = default;

private slots:
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setupUi();
    void populateTable(const QStringList &lines);

    // ── Header ────────────────────────────────────────────────────────────────
    QLabel       *m_countLabel  = nullptr;

    // ── Table ─────────────────────────────────────────────────────────────────
    QTableWidget *m_table       = nullptr;

    // ── Buttons ───────────────────────────────────────────────────────────────
    QPushButton  *m_btnInstall  = nullptr;
    QPushButton  *m_btnCancel   = nullptr;

    // ── Process ───────────────────────────────────────────────────────────────
    QProcess    *m_process      = nullptr;
    QString      m_buf;
    QStringList  m_targets;
};