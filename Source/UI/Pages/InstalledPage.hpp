#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QHeaderView>

class InstalledPage : public QWidget
{
    Q_OBJECT

public:
    explicit InstalledPage(QWidget *parent = nullptr);
    ~InstalledPage() override = default;

    void updateIcons(bool isDark);

private:
    void setupUi();
    void populateDemoRows();

    QLineEdit    *m_searchInput   = nullptr;
    QPushButton  *m_btnSearch     = nullptr;
    QPushButton  *m_btnClear      = nullptr;

    QPushButton  *m_btnRemove     = nullptr;   // "Remove Selected"
    QPushButton  *m_btnSelAll     = nullptr;
    QPushButton  *m_btnClrSel     = nullptr;

    QTableWidget *m_table         = nullptr;

    QLabel       *m_outputIcon    = nullptr;
    QLabel       *m_outputLabel   = nullptr;
    QPlainTextEdit *m_outputView  = nullptr;
};