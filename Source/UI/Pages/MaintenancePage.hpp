#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QProcess>
#include <QList>
#include <functional>

// MaintenancePage
// ---------------
// Provides grouped maintenance tasks in a left scrollable panel with a live
// Maintenance Log and auto-populated Orphaned Packages panel on the right.
//
// Uses its own QProcess rather than PacmanBackend because maintenance spans
// a heterogeneous set of tools: paccache, reflector, find, tar, pacman.
//
// Privileged operations are dispatched via pkexec.
// Unprivileged ones run directly.

class MaintenancePage : public QWidget
{
    Q_OBJECT

public:
    explicit MaintenancePage(QWidget *parent = nullptr);
    ~MaintenancePage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    // ── Package Cache ─────────────────────────────────────────────────────────
    void onClearPackageCache();
    void onClearAllCache();

    // ── Database Maintenance ──────────────────────────────────────────────────
    void onCheckDatabaseIntegrity();
    void onBackupDatabase();
    void onRestoreDatabase();
    void onRankMirrors();

    // ── Configuration Management ──────────────────────────────────────────────
    void onFindPacnewFiles();

    // ── System Maintenance ────────────────────────────────────────────────────
    void onCheckIntegrity();
    void onClearPackageLock();

    // ── Orphan Packages ───────────────────────────────────────────────────────
    void onRemoveOrphans();

    // ── QProcess callbacks ────────────────────────────────────────────────────
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setupUi();
    void setupTaskPanel(QVBoxLayout *layout);

    QWidget *makeSectionHeader(const QString &icon, const QString &title);
    QWidget *makeTaskCard(const QString         &title,
                          const QString         &description,
                          std::function<void()>  callback);

    void runPrivileged(const QStringList &argv);
    void runUnprivileged(const QStringList &argv);

    void queryOrphans();
    void showLog();
    void appendLog(const QString &line);
    void setTasksBusy(bool busy);
    [[nodiscard]] bool isBusy() const;

    // ── Right panel ───────────────────────────────────────────────────────────
    QWidget        *m_placeholder  = nullptr;   // shown before first operation
    QPlainTextEdit *m_logView      = nullptr;
    QLabel         *m_orphanLabel  = nullptr;   // "Found N orphaned package(s):"
    QLabel         *m_orphanList   = nullptr;   // comma-separated names

    // ── Process ──────────────────────────────────────────────────────────────
    QProcess       *m_process      = nullptr;
    QString         m_lineBuf;                  // partial-line accumulator

    enum class Op {
        None,
        QueryOrphans,
        ClearCache,
        ClearAllCache,
        CheckDbIntegrity,
        BackupDb,
        RestoreDb,
        RankMirrors,
        FindPacnew,
        CheckIntegrity,
        ClearLock,
        RemoveOrphans
    };
    Op m_currentOp = Op::None;

    // All task-card buttons — bulk-disabled during any running operation
    QList<QPushButton *> m_taskButtons;
};