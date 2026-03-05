#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QProcess>
#include <QList>

// KernelPage
// ----------
// Displays available and installed kernels via `pacman -Ss ^linux`.
// Installed status is determined by cross-referencing `pacman -Q`.
//
// Layout (matching screenshot):
//   Full-width table: Name | Version | Repository | Description | Status
//   Footer: [Install Selected]  [Remove Selected]
//   Bottom pane: Terminal Output (placeholder until operation runs)
//
// Clicking "Install Selected" opens KernelInstallDialog for dep preview,
// then runs pkexec pacman -S on confirm.
// Clicking "Remove Selected" shows a confirmation then runs pkexec pacman -Rns.
//
// Rows are multi-selectable via QTableWidget row selection (no extra checkbox
// column — matches the screenshot).

struct KernelPackage
{
    QString name;
    QString version;
    QString repo;
    QString description;
    bool    installed = false;
};

class KernelPage : public QWidget
{
    Q_OBJECT

public:
    explicit KernelPage(QWidget *parent = nullptr);
    ~KernelPage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    void onInstallSelected();
    void onRemoveSelected();
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setupUi();
    void loadKernels();
    void populateTable(const QList<KernelPackage> &kernels);
    void appendOutput(const QString &line);
    void setButtonsBusy(bool busy);

    void parseInstalledOutput(const QString &raw);
    void parseSearchOutput(const QString &raw);

    enum class Op { None, QueryInstalled, Search, Install, Remove };
    Op m_currentOp = Op::None;

    QList<KernelPackage> m_kernels;
    QStringList          m_installedNames;
    QString              m_outputBuf;

    // ── UI ────────────────────────────────────────────────────────────────────
    QTableWidget   *m_table    = nullptr;
    QPushButton    *m_btnInstall = nullptr;
    QPushButton    *m_btnRemove  = nullptr;
    QPlainTextEdit *m_outputView = nullptr;

    // ── Process ───────────────────────────────────────────────────────────────
    QProcess *m_process = nullptr;
};