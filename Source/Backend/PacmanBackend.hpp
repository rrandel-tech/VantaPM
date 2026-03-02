#pragma once

#include "Package.hpp"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class PacmanBackend : public QObject
{
    Q_OBJECT

public:
    explicit PacmanBackend(QObject *parent = nullptr);

    // ── Read-only queries ─────────────────────────────────────────────────────
    void search(const QString &term);
    void queryInstalled();
    void queryInstalledFull();
    void queryExplicit();
    void queryUpgradable();
    void infoLocal(const QString &package);
    void infoSync(const QString &package);

    // Sync databases with pkexec (prompts polkit) then emit upgradablePackages
    // with full current+new version info.
    void checkUpdates();

    // ── Write operations (pkexec → polkit auth) ───────────────────────────────
    void install(const QStringList &packages);
    void remove(const QStringList &packages);
    void sysUpgrade();
    void syncDatabases();

    bool isBusy() const;

    signals:
        void outputLine(const QString &line);
    void finished(bool success, int exitCode);
    void startError(const QString &message);

    void searchResults(const QList<Package> &packages);
    void queryResults(const QList<Package> &packages);
    void upgradableResults(const QStringList &packageNames);

    // Emitted by checkUpdates() with full old+new version info
    void upgradablePackages(const QList<Package> &packages);

private:
    enum class OutputMode { Raw, Search, Query, QueryInfo, Upgradable, UpgradableFull };

    void run(const QStringList &argv, bool withPrivilege,
             OutputMode mode = OutputMode::Raw);

    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus status);

    QProcess   *m_process    = nullptr;
    QString     m_lineBuf;
    QString     m_fullBuf;
    OutputMode  m_outputMode = OutputMode::Raw;
};