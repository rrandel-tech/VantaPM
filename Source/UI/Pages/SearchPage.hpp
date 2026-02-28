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

    void appendOutput(const QString &line);
    void updateIcons(bool isDark);

private:
    void setupUi();

    QLabel         *m_emptyIcon = nullptr;

    QLineEdit      *m_searchInput    = nullptr;
    QComboBox      *m_repoFilter     = nullptr;
    QComboBox      *m_statusFilter   = nullptr;
    QCheckBox      *m_includeAur     = nullptr;
    QRadioButton   *m_installedOnly  = nullptr;
    QLabel         *m_emptyLabel     = nullptr;

    QPushButton    *m_btnSearch      = nullptr;
    QPushButton    *m_btnClear       = nullptr;
    QPushButton    *m_btnInstall     = nullptr;
    QPushButton    *m_btnRemove      = nullptr;
    QPushButton    *m_btnSelAll      = nullptr;
    QPushButton    *m_btnClrSel      = nullptr;

    QTabWidget     *m_bottomTabs     = nullptr;
    TerminalWidget *m_terminal       = nullptr;
    QPlainTextEdit *m_outputView     = nullptr;
};