#pragma once

#include "Package.hpp"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

// PacmanBackend
// -------------
// Async wrapper around the pacman binary.
//
// Read-only operations  → run directly as the current user.
// Write operations      → run under pkexec; the polkit agent prompts for auth.
//
// All output is emitted line-by-line via outputLine().
// Never calls system(). Never interpolates user input into a shell string.

class PacmanBackend : public QObject
{
    Q_OBJECT

public:
    explicit PacmanBackend(QObject *parent = nullptr);

    // ── Read-only queries ─────────────────────────────────────────────────────
    void search(const QString &term);

    // Fast: emits queryResults with name+version only (no repo/description)
    void queryInstalled();

    // Full: runs `pacman -Qi` on all installed packages.
    // Slower but populates repo and description.
    // Emits queryResults with complete Package structs.
    void queryInstalledFull();

    void queryExplicit();
    void queryUpgradable();
    void infoLocal(const QString &package);
    void infoSync(const QString &package);

    // ── Write operations (pkexec → polkit auth prompt) ────────────────────────
    void install(const QStringList &packages);
    void remove(const QStringList &packages);
    void sysUpgrade();
    void syncDatabases();

    // True while a child process is running
    bool isBusy() const;

signals:
    // One complete stdout/stderr line at a time
    void outputLine(const QString &line);

    // Emitted when the process exits
    void finished(bool success, int exitCode);

    // Emitted if the process cannot be started
    void startError(const QString &message);

    // Parsed signals
    void searchResults(const QList<Package> &packages);
    void queryResults(const QList<Package> &packages);
    void upgradableResults(const QStringList &packageNames);

private:
    enum class OutputMode { Raw, Search, Query, QueryInfo, Upgradable };

    void run(const QStringList &argv, bool withPrivilege,
             OutputMode mode = OutputMode::Raw);

    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus status);

    QProcess   *m_process    = nullptr;
    QString     m_lineBuf;
    QString     m_fullBuf;
    OutputMode  m_outputMode = OutputMode::Raw;
};