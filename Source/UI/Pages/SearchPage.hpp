#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QTabWidget>
#include <QPlainTextEdit>

class TerminalWidget;

class SearchPage : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPage(QWidget *parent = nullptr);
    ~SearchPage() override = default;

    // Called by the backend to append a line to the Output tab
    void appendOutput(const QString &line);

private:
    void setupUi();

    QLineEdit      *m_searchInput    = nullptr;
    QComboBox      *m_repoFilter     = nullptr;
    QComboBox      *m_statusFilter   = nullptr;
    QCheckBox      *m_includeAur     = nullptr;
    QRadioButton   *m_installedOnly  = nullptr;
    QLabel         *m_emptyLabel     = nullptr;

    // Bottom panel
    QTabWidget     *m_bottomTabs     = nullptr;
    TerminalWidget *m_terminal       = nullptr;   // Tab 0 – real PTY terminal
    QPlainTextEdit *m_outputView     = nullptr;   // Tab 1 – append-only backend log
};