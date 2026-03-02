#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QList>

#include "Backend/Package.hpp"

class PacmanBackend;

class UpdatePage : public QWidget
{
    Q_OBJECT

public:
    explicit UpdatePage(QWidget *parent = nullptr);
    ~UpdatePage() override = default;

    void updateIcons(bool isDark);

signals:
    void statusMessage(const QString &message);

private slots:
    void onCheckUpdates();
    void onUpdateSystem();
    void onUpgradablePackages(const QList<Package> &packages);
    void onOutputLine(const QString &line);
    void onFinished(bool success, int exitCode);

private:
    void setupUi();
    void setEmptyState(const QString &message);
    void populateTable(const QList<Package> &packages);
    void appendOutput(const QString &line);
    void setButtonsBusy(bool busy);

    QList<Package> m_upgradable;

    // ── Top bar ───────────────────────────────────────────────────────────────
    QPushButton    *m_btnCheck      = nullptr;
    QPushButton    *m_btnUpdate     = nullptr;
    QPushButton    *m_btnAurCheck   = nullptr;  // stub — no-op
    QPushButton    *m_btnAurUpdate  = nullptr;  // stub — no-op

    // ── Status bar ────────────────────────────────────────────────────────────
    QLabel         *m_statusIcon    = nullptr;
    QLabel         *m_statusLabel   = nullptr;

    // ── Package table (hidden when empty) ─────────────────────────────────────
    QTableWidget   *m_table         = nullptr;

    // ── Empty state (hidden when table is shown) ──────────────────────────────
    QWidget        *m_emptyWidget   = nullptr;
    QLabel         *m_emptyLabel    = nullptr;
    QLabel         *m_emptyHint     = nullptr;

    // ── Output pane ───────────────────────────────────────────────────────────
    QLabel         *m_outputLabel   = nullptr;
    QPlainTextEdit *m_outputView    = nullptr;
    QPushButton    *m_btnClearOut   = nullptr;

    // ── Backend ──────────────────────────────────────────────────────────────
    PacmanBackend  *m_backend       = nullptr;

    enum class Op { None, Check, Upgrade };
    Op m_currentOp = Op::None;
};