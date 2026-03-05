#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QProcess>
#include <QSplitter>
#include <QStackedWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>

// FlatpakPage
// -----------
// Provides Flatpak search, install, and management via the `flatpak` CLI.
// Layout:
//   Left:  search bar → search results area (empty-state or table) →
//          Install Selected → Installed Packages section (filterable table)
//   Right: detail pane — placeholder when nothing selected;
//          Application Details + Permissions + Actions when a row is clicked.
//
// All operations are non-blocking (QProcess). flatpak handles its own
// privilege escalation internally so no pkexec wrapper is needed.

struct FlatpakPackage
{
    QString name;
    QString appId;
    QString version;
    QString branch;
    QString origin;
    QString installation; // "system" | "user"
    QString runtime;
    QString description;
    QString permissions;  // newline-joined from flatpak info
    bool    installed = false;
};

class FlatpakPage : public QWidget
{
    Q_OBJECT

public:
    explicit FlatpakPage(QWidget *parent = nullptr);
    ~FlatpakPage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    // ── Search panel ──────────────────────────────────────────────────────────
    void onSearch();
    void onSearchRowClicked(int row, int col);
    void onInstallSelected();

    // ── Installed panel ───────────────────────────────────────────────────────
    void onFilterInstalled(const QString &text);
    void onInstalledRowClicked(int row, int col);

    // ── Detail-pane actions ───────────────────────────────────────────────────
    void onManageUserData();
    void onUninstall();
    void onRemoveData();
    void onCreateSnapshot();
    void onRestoreSnapshot();

    // ── Process callbacks ─────────────────────────────────────────────────────
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    // ── UI construction ───────────────────────────────────────────────────────
    void setupUi();
    QWidget *buildDetailPane();
    QWidget *buildActionsSection();

    // ── Helpers ───────────────────────────────────────────────────────────────
    void runFlatpak(const QStringList &args);
    bool isBusy() const;

    void populateSearchTable(const QList<FlatpakPackage> &pkgs);
    void populateInstalledTable(const QList<FlatpakPackage> &pkgs);

    void showDetailPlaceholder();
    void showDetailForPackage(const FlatpakPackage &pkg);
    void refreshInstalledList();

    void parseSearchOutput(const QString &raw);
    void parseInstalledOutput(const QString &raw);

    // ── State ─────────────────────────────────────────────────────────────────
    enum class Op { None, Search, ListInstalled, FetchInfo, Install, Uninstall, RemoveData };
    Op      m_currentOp   = Op::None;
    QString m_pendingAppId;
    QString m_selectedAppId;

    QList<FlatpakPackage> m_searchResults;
    QList<FlatpakPackage> m_installedPackages;
    QString               m_outputBuf;

    // ── Left panel ────────────────────────────────────────────────────────────
    QLineEdit      *m_searchInput        = nullptr;
    QPushButton    *m_btnSearch          = nullptr;

    // Search results: stacked between empty-state and table
    QStackedWidget *m_searchStack        = nullptr; // 0 = empty state, 1 = table
    QLabel         *m_searchEmptyLabel   = nullptr;
    QTableWidget   *m_searchTable        = nullptr;

    QPushButton    *m_btnInstallSelected = nullptr;

    QLineEdit      *m_installedFilter    = nullptr;
    QTableWidget   *m_installedTable     = nullptr;

    // ── Right panel ───────────────────────────────────────────────────────────
    QWidget        *m_detailPane         = nullptr;
    QStackedWidget *m_detailStack        = nullptr; // 0 = placeholder, 1 = details

    // Detail labels
    QLabel *m_detailName        = nullptr;
    QLabel *m_detailVersion     = nullptr;
    QLabel *m_detailBranch      = nullptr;
    QLabel *m_detailOrigin      = nullptr;
    QLabel *m_detailInstall     = nullptr;
    QLabel *m_detailRuntime     = nullptr;
    QLabel *m_detailDescription = nullptr;
    QLabel *m_detailPermissions = nullptr;

    // Action buttons
    QPushButton *m_btnManageData      = nullptr;
    QPushButton *m_btnUninstall       = nullptr;
    QPushButton *m_btnRemoveData      = nullptr;
    QPushButton *m_btnCreateSnapshot  = nullptr;
    QPushButton *m_btnRestoreSnapshot = nullptr;

    // ── Process ───────────────────────────────────────────────────────────────
    QProcess *m_process = nullptr;
};