#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QHeaderView>
#include <QCheckBox>
#include <QList>

#include "Backend/Package.hpp"

class PacmanBackend;

class InstalledPage : public QWidget
{
    Q_OBJECT

public:
    explicit InstalledPage(QWidget *parent = nullptr);
    ~InstalledPage() override = default;

    void loadPackages();
    void updateIcons(bool isDark);

private slots:
    void onSearch();
    void onClear();
    void onRemoveSelected();
    void onSelectAll();
    void onClearSelection();

    void onQueryResults(const QList<Package> &packages);
    void onOutputLine(const QString &line);
    void onFinished(bool success, int exitCode);

private:
    void setupUi();
    void populateTable(const QList<Package> &packages);
    void applySearch(const QString &term);
    void appendOutput(const QString &line);

    QList<Package> m_allPackages;

    enum class Op { None, Query, Remove, Info };
    Op m_currentOp = Op::None;

    bool m_isDark = true;

    QLineEdit      *m_searchInput  = nullptr;
    QPushButton    *m_btnSearch    = nullptr;
    QPushButton    *m_btnClear     = nullptr;
    QPushButton    *m_btnRemove    = nullptr;
    QPushButton    *m_btnSelAll    = nullptr;
    QPushButton    *m_btnClrSel    = nullptr;

    QTableWidget   *m_table        = nullptr;

    QLabel         *m_outputIcon   = nullptr;
    QPlainTextEdit *m_outputView   = nullptr;

    PacmanBackend  *m_backend      = nullptr;
};