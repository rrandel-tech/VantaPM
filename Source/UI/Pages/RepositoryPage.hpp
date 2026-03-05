#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QProcess>
#include <QStackedWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>

// RepositoryPage
// --------------
// Reads and manages pacman repositories defined in /etc/pacman.conf.
//
// Layout:
//   Left:  "Repositories" header + Refresh → table (Repository, Server/Include,
//          Status, SigLevel) → Add / Edit / Remove footer buttons
//   Right: "Repository Details" panel — placeholder until a row is selected,
//          then shows name, server/include, status, siglevel, and usage stats.
//
// Write operations (add, edit, remove) modify /etc/pacman.conf via pkexec
// so that privilege escalation is consistent with the rest of VantaPM.
// Reads are done directly (no privilege needed for /etc/pacman.conf).

struct Repository
{
    QString name;
    QString server;   // Server= or Include= value (first one found)
    QString sigLevel; // SigLevel= value, empty if not set
    bool    enabled = true;  // false if the [section] is commented out
};

class RepositoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit RepositoryPage(QWidget *parent = nullptr);
    ~RepositoryPage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    void onRefresh();
    void onRowClicked(int row, int col);
    void onAdd();
    void onEdit();
    void onRemove();

    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void setupUi();
    QWidget *buildDetailPane();

    // ── pacman.conf helpers ───────────────────────────────────────────────────
    void loadFromPacmanConf();
    void populateTable(const QList<Repository> &repos);
    void showDetailPlaceholder();
    void showDetailForRepo(const Repository &repo);

    // Privilege-escalated write via pkexec + sed/tee
    void runPrivileged(const QStringList &argv);
    bool isBusy() const;

    // ── State ─────────────────────────────────────────────────────────────────
    enum class Op { None, Add, Edit, Remove };
    Op  m_currentOp  = Op::None;
    int m_selectedRow = -1;

    QList<Repository> m_repos;
    QString           m_outputBuf;
    QString           m_pendingWriteData;

    // ── Left panel ────────────────────────────────────────────────────────────
    QTableWidget *m_table      = nullptr;
    QPushButton  *m_btnRefresh = nullptr;
    QPushButton  *m_btnAdd     = nullptr;
    QPushButton  *m_btnEdit    = nullptr;
    QPushButton  *m_btnRemove  = nullptr;

    // ── Right panel ───────────────────────────────────────────────────────────
    QStackedWidget *m_detailStack = nullptr; // 0 = placeholder, 1 = details

    QLabel *m_detailName    = nullptr;
    QLabel *m_detailServer  = nullptr;
    QLabel *m_detailStatus  = nullptr;
    QLabel *m_detailSigLevel= nullptr;

    // ── Process ───────────────────────────────────────────────────────────────
    QProcess *m_process = nullptr;
};