#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>

class SearchPage;
class InstalledPage;
class UpdatePage;
class MaintenancePage;
class FlatpakPage;
class RepositoryPage;
class KernelPage;
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
    void toggleTheme();
    void updateIcons();

    [[nodiscard]] QString iconPath(const QString &name) const;

    bool m_isDark = true;

    QStackedWidget  *m_pageStack         = nullptr;

    QPushButton     *m_btnSearch         = nullptr;
    QPushButton     *m_btnInstalled      = nullptr;
    QPushButton     *m_btnSysUpdate      = nullptr;
    QPushButton     *m_btnMaintenance    = nullptr;
    QPushButton     *m_btnFlatpak        = nullptr;
    QPushButton     *m_btnRepository     = nullptr;
    QPushButton     *m_btnKernel         = nullptr;
    QPushButton     *m_btnTheme          = nullptr;
    QPushButton     *m_btnSettings       = nullptr;

    SearchPage      *m_searchPage        = nullptr;
    InstalledPage   *m_installedPage     = nullptr;
    UpdatePage      *m_updatePage        = nullptr;
    MaintenancePage *m_maintenancePage   = nullptr;
    FlatpakPage     *m_flatpakPage       = nullptr;
    RepositoryPage  *m_repositoryPage    = nullptr;
    KernelPage      *m_kernelPage        = nullptr;
    SettingsDialog  *m_settingsDialog    = nullptr;
};