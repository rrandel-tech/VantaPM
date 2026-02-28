#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>

class SearchPage;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void setupUi();
    void applyStyleSheet();

    QStackedWidget *m_pageStack       = nullptr;

    QPushButton    *m_btnSearch       = nullptr;
    QPushButton    *m_btnInstalled    = nullptr;
    QPushButton    *m_btnSysUpdate    = nullptr;
    QPushButton    *m_btnMaintenance  = nullptr;
    QPushButton    *m_btnFlatpak      = nullptr;
    QPushButton    *m_btnRepository   = nullptr;
    QPushButton    *m_btnKernel       = nullptr;

    SearchPage     *m_searchPage      = nullptr;
    SettingsDialog  *m_settingsDialog  = nullptr;
};