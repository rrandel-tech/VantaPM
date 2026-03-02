#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QList>

#include "Backend/Package.hpp"

class PacmanBackend;

class SearchPage : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPage(QWidget *parent = nullptr);
    ~SearchPage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    void onSearch();
    void onClear();
    void onInstallSelected();
    void onRemoveSelected();
    void onSelectAll();
    void onClearSelection();
    void onInstalledOnlyToggled(bool checked);
    void onRepoFilterChanged(int index);
    void onStatusFilterChanged(int index);

    void onSearchResults(const QList<Package> &packages);
    void onOutputLine(const QString &line);
    void onFinished(bool success, int exitCode);

private:
    void setupUi();
    void populateTable(const QList<Package> &packages);
    void applyFilters();
    void appendOutput(const QString &line);

    QList<Package>  m_allResults;   // full unfiltered result set from last search

    // ── Top controls ─────────────────────────────────────────────────────────
    QLabel         *m_emptyIcon     = nullptr;
    QLineEdit      *m_searchInput   = nullptr;
    QComboBox      *m_repoFilter    = nullptr;
    QComboBox      *m_statusFilter  = nullptr;
    QCheckBox      *m_includeAur    = nullptr;
    QRadioButton   *m_installedOnly = nullptr;
    QLabel         *m_emptyLabel    = nullptr;

    QPushButton    *m_btnSearch     = nullptr;
    QPushButton    *m_btnClear      = nullptr;
    QPushButton    *m_btnInstall    = nullptr;
    QPushButton    *m_btnRemove     = nullptr;
    QPushButton    *m_btnSelAll     = nullptr;
    QPushButton    *m_btnClrSel     = nullptr;

    // ── Package table ─────────────────────────────────────────────────────────
    QTableWidget   *m_table         = nullptr;
    QWidget        *m_emptyState    = nullptr;   // shown when table is empty
    QWidget        *m_tableWrapper  = nullptr;   // contains m_table

    // ── Output pane ───────────────────────────────────────────────────────────
    QPlainTextEdit *m_outputView    = nullptr;

    // ── Backend ──────────────────────────────────────────────────────────────
    PacmanBackend  *m_backend       = nullptr;
};