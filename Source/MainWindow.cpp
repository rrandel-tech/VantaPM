#include "MainWindow.hpp"
#include "UI/Pages/SearchPage.hpp"
#include "UI/Pages/InstalledPage.hpp"
#include "UI/Pages/UpdatePage.hpp"
#include "UI/Pages/MaintenancePage.hpp"
#include "UI/Pages/FlatpakPage.hpp"
#include "UI/Pages/RepositoryPage.hpp"
#include "UI/Pages/KernelPage.hpp"
#include "UI/SettingsDialog.hpp"
#include "SettingsManager.hpp"

#include <QApplication>
#include <QStatusBar>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("VantaPM");
    setMinimumSize(1100, 700);
    resize(1456, 816);

    m_isDark = (SettingsManager::instance().theme() == "dark");

    setupUi();
    applyStyleSheet();
    updateIcons();

    statusBar()->showMessage("  Ready");

    connect(m_searchPage, &SearchPage::statusMessage,
            this, [this](const QString &msg) { statusBar()->showMessage(msg); });

    connect(m_installedPage, &InstalledPage::statusMessage,
            this, [this](const QString &msg) {
        if (m_pageStack->currentIndex() == 1)
            statusBar()->showMessage(msg);
    });

    connect(m_updatePage, &UpdatePage::statusMessage,
            this, [this](const QString &msg) { statusBar()->showMessage(msg); });

    connect(m_maintenancePage, &MaintenancePage::statusMessage,
            this, [this](const QString &msg) {
        if (m_pageStack->currentIndex() == 3)
            statusBar()->showMessage(msg);
    });

    connect(m_flatpakPage, &FlatpakPage::statusMessage,
            this, [this](const QString &msg) {
        if (m_pageStack->currentIndex() == 4)
            statusBar()->showMessage(msg);
    });

    connect(m_repositoryPage, &RepositoryPage::statusMessage,
            this, [this](const QString &msg) {
        if (m_pageStack->currentIndex() == 5)
            statusBar()->showMessage(msg);
    });

    connect(m_kernelPage, &KernelPage::statusMessage,
            this, [this](const QString &msg) {
        if (m_pageStack->currentIndex() == 6)
            statusBar()->showMessage(msg);
    });

    connect(m_settingsDialog, &SettingsDialog::themeChanged,
            this, [this](const QString &theme) {
        m_isDark = (theme == "dark");
        applyStyleSheet();
        updateIcons();
        m_searchPage->updateIcons(m_isDark);
        m_installedPage->updateIcons(m_isDark);
        m_updatePage->updateIcons(m_isDark);
        m_maintenancePage->updateIcons(m_isDark);
        m_flatpakPage->updateIcons(m_isDark);
        m_repositoryPage->updateIcons(m_isDark);
        m_kernelPage->updateIcons(m_isDark);
    });

    connect(&SettingsManager::instance(), &SettingsManager::autoRefreshTriggered,
            this, [this]() {
        m_installedPage->loadPackages();
        SettingsManager::instance().notify(
            QStringLiteral("VantaPM"),
            QStringLiteral("Package list refreshed"));
    });

    QTimer::singleShot(0, this, [this]() {
        m_installedPage->loadPackages();
    });
}

QString MainWindow::iconPath(const QString &name) const
{
    return QString(":/icons/%1/%2").arg(m_isDark ? "light" : "dark", name);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Nav bar ───────────────────────────────────────────────────────────────
    auto *navBar = new QWidget;
    navBar->setObjectName("navBar");
    navBar->setFixedHeight(50);

    auto *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(10, 6, 10, 6);
    navLayout->setSpacing(2);

    auto makeNavBtn = [](const QString &label) {
        auto *btn = new QPushButton(label);
        btn->setIconSize(QSize(16, 16));
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    m_btnSearch      = makeNavBtn("Search");
    m_btnInstalled   = makeNavBtn("Installed");
    m_btnSysUpdate   = makeNavBtn("System Update");
    m_btnMaintenance = makeNavBtn("Maintenance");
    m_btnFlatpak     = makeNavBtn("Flatpak");
    m_btnRepository  = makeNavBtn("Repository");
    m_btnKernel      = makeNavBtn("Kernel");
    m_btnSearch->setChecked(true);

    for (auto *btn : {m_btnSearch, m_btnInstalled, m_btnSysUpdate,
                      m_btnMaintenance, m_btnFlatpak, m_btnRepository, m_btnKernel})
        navLayout->addWidget(btn);

    navLayout->addStretch();

    m_btnTheme    = new QPushButton;
    m_btnSettings = new QPushButton;
    m_btnTheme->setIconSize(QSize(18, 18));
    m_btnSettings->setIconSize(QSize(18, 18));
    m_btnTheme->setObjectName("iconBtn");
    m_btnSettings->setObjectName("iconBtn");
    m_btnTheme->setFixedSize(34, 34);
    m_btnSettings->setFixedSize(34, 34);
    m_btnTheme->setToolTip("Toggle Theme");
    m_btnSettings->setToolTip("Settings");

    navLayout->addWidget(m_btnTheme);
    navLayout->addWidget(m_btnSettings);

    m_settingsDialog = new SettingsDialog(this);

    connect(m_btnTheme, &QPushButton::clicked, this, [this]() {
        toggleTheme();
        m_settingsDialog->syncTheme(m_isDark);
    });

    connect(m_btnSettings, &QPushButton::clicked, this, [this]() {
        m_settingsDialog->exec();
    });

    rootLayout->addWidget(navBar);

    // ── Page stack ────────────────────────────────────────────────────────────
    m_pageStack = new QStackedWidget;

    m_searchPage = new SearchPage;
    m_pageStack->addWidget(m_searchPage);           // index 0

    m_installedPage = new InstalledPage;
    m_pageStack->addWidget(m_installedPage);        // index 1

    m_updatePage = new UpdatePage;
    m_pageStack->addWidget(m_updatePage);           // index 2

    m_maintenancePage = new MaintenancePage;
    m_pageStack->addWidget(m_maintenancePage);      // index 3

    m_flatpakPage = new FlatpakPage;
    m_pageStack->addWidget(m_flatpakPage);          // index 4

    m_repositoryPage = new RepositoryPage;
    m_pageStack->addWidget(m_repositoryPage);       // index 5

    m_kernelPage = new KernelPage;
    m_pageStack->addWidget(m_kernelPage);           // index 6


    auto *contentWrapper = new QWidget;
    contentWrapper->setObjectName("contentWrapper");
    auto *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->addWidget(m_pageStack);
    rootLayout->addWidget(contentWrapper, 1);

    const QList<QPushButton *> navBtns = {
        m_btnSearch, m_btnInstalled, m_btnSysUpdate, m_btnMaintenance,
        m_btnFlatpak, m_btnRepository, m_btnKernel
    };

    const QStringList pageDefaults = {
        "  Search packages",
        "  Installed packages",
        "  System update",
        "  Maintenance",
        "  Flatpak",
        "  Repository",
        "  Kernel"
    };

    for (int i = 0; i < navBtns.size(); ++i) {
        connect(navBtns[i], &QPushButton::clicked, this, [this, navBtns, pageDefaults, i]() {
            m_pageStack->setCurrentIndex(i);
            for (int j = 0; j < navBtns.size(); ++j)
                navBtns[j]->setChecked(j == i);

            if (i == 1 && m_installedPage->packageCount() > 0)
                statusBar()->showMessage(
                    QStringLiteral("  Found %1 installed package(s)")
                        .arg(m_installedPage->packageCount()));
            else
                statusBar()->showMessage(pageDefaults[i]);
        });
    }
}

void MainWindow::toggleTheme()
{
    m_isDark = !m_isDark;
    applyStyleSheet();
    updateIcons();
    m_searchPage->updateIcons(m_isDark);
    m_installedPage->updateIcons(m_isDark);
    m_updatePage->updateIcons(m_isDark);
    m_maintenancePage->updateIcons(m_isDark);
    m_flatpakPage->updateIcons(m_isDark);
    m_repositoryPage->updateIcons(m_isDark);
    m_kernelPage->updateIcons(m_isDark);
}

void MainWindow::updateIcons()
{
    m_btnSearch->setIcon(QIcon(iconPath("search.png")));
    m_btnInstalled->setIcon(QIcon(iconPath("installed.png")));
    m_btnSysUpdate->setIcon(QIcon(iconPath("update.png")));
    m_btnMaintenance->setIcon(QIcon(iconPath("maintenance.png")));
    m_btnFlatpak->setIcon(QIcon(iconPath("flatpak.png")));
    m_btnRepository->setIcon(QIcon(iconPath("repository.png")));
    m_btnKernel->setIcon(QIcon(iconPath("kernel.png")));
    m_btnTheme->setIcon(QIcon(iconPath("theme.png")));
    m_btnSettings->setIcon(QIcon(iconPath("settings.png")));
}

void MainWindow::applyStyleSheet()
{
    static bool styleSet = false;
    if (!styleSet) {
        qApp->setStyle("Fusion");
        styleSet = true;
    }

    const QString bg          = m_isDark ? "#1e1e1e" : "#f5f5f5";
    const QString bgNav       = m_isDark ? "#252526" : "#e8e8e8";
    const QString bgWidget    = m_isDark ? "#2a2a2a" : "#ffffff";
    const QString bgHover     = m_isDark ? "#2a2a2a" : "#e0e0e0";
    const QString bgChecked   = m_isDark ? "#37373d" : "#d6d6d6";
    const QString bgTerminal  = m_isDark ? "#1a1a1a" : "#ffffff";
    const QString border      = m_isDark ? "#2d2d2d" : "#d0d0d0";
    const QString borderInput = m_isDark ? "#383838" : "#c0c0c0";
    const QString textPrimary = m_isDark ? "#d4d4d4" : "#1e1e1e";
    const QString textMuted   = m_isDark ? "#999"    : "#666";
    const QString headerBg     = m_isDark ? "rgba(0,120,212,0.10)" : "rgba(0,120,212,0.08)";
    const QString headerFg     = m_isDark ? "#7ec8f0" : "#005a9e";
    const QString headerBorder = m_isDark ? "rgba(0,120,212,0.25)" : "rgba(0,120,212,0.20)";

    const QString qss =
        "QMainWindow { background-color: %1; color: %2; font-family: 'Noto Sans', 'Segoe UI', sans-serif; font-size: 13px; }"
        "#contentWrapper { background-color: %1; color: %2; font-family: 'Noto Sans', 'Segoe UI', sans-serif; font-size: 13px; }"
        "#navBar { background-color: %3; border-bottom: 1px solid %8; }"

        "QPushButton#navBtn { background-color: transparent; color: %10; border: none; border-radius: 5px; padding: 5px 14px; font-size: 13px; }"
        "QPushButton#navBtn:hover { background-color: %5; color: %2; }"
        "QPushButton#navBtn:checked { background-color: %6; color: %2; border-bottom: 2px solid #0078d4; }"
        "QPushButton#iconBtn { background-color: transparent; border: none; border-radius: 5px; }"
        "QPushButton#iconBtn:hover { background-color: %5; }"

        "QLabel#pageTitle { font-size: 15px; font-weight: 600; color: %2; }"

        "QLineEdit#searchInput { background-color: %4; border: 1px solid %9; border-radius: 5px; padding: 6px 10px; color: %2; }"
        "QLineEdit#searchInput:focus { border-color: #0078d4; }"

        "QPushButton#btnPrimary { background-color: #0078d4; color: #fff; border: none; border-radius: 5px; padding: 0 16px; }"
        "QPushButton#btnPrimary:hover { background-color: #1a8de0; }"
        "QPushButton#btnPrimary:pressed { background-color: #006cbf; }"
        "QPushButton#btnSecondary { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; }"
        "QPushButton#btnSecondary:hover { background-color: %5; }"
        "QPushButton#btnInstall { background-color: #1e6b3a; color: #d4d4d4; border: none; border-radius: 5px; padding: 0 16px; }"
        "QPushButton#btnInstall:hover { background-color: #237a43; }"
        "QPushButton#btnRemove { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; }"
        "QPushButton#btnRemove:hover { background-color: #3a1e1e; border-color: #5a2a2a; color: #d4d4d4; }"

        "QLabel#sectionLabel { color: %10; font-size: 12px; font-weight: 600; }"
        "QLabel#filterLabel  { color: %10; }"
        "QLabel#pkgName      { font-size: 13px; color: %2; }"
        "QLabel#pkgVersion   { color: %10; font-size: 12px; }"
        "QLabel#pkgDesc      { color: %10; font-size: 12px; }"
        "QLabel#emptyLabel   { color: %2; font-size: 14px; margin-top: 4px; }"
        "QLabel#emptyHint    { color: %2; font-size: 12px; }"
        "QLabel#statusInstalled    { color: #4ec994; font-size: 12px; }"
        "QLabel#statusNotInstalled { color: %10; font-size: 12px; }"

        "QComboBox#filterCombo { background-color: %4; border: 1px solid %9; border-radius: 4px; padding: 4px 8px; color: %2; min-width: 140px; }"
        "QComboBox#filterCombo::drop-down { border: none; width: 20px; }"
        "QComboBox#filterCombo QAbstractItemView { background-color: %3; border: 1px solid %9; color: %2; selection-background-color: #0078d4; }"

        "QCheckBox  { color: %2; spacing: 6px; }"
        "QRadioButton { color: %2; spacing: 6px; }"
        "QCheckBox::indicator  { width: 15px; height: 15px; border: 1px solid %10; border-radius: 3px; background: %4; }"
        "QRadioButton::indicator { width: 15px; height: 15px; border: 1px solid %10; border-radius: 7px; background: %4; }"
        "QCheckBox::indicator:checked   { background: #0078d4; border-color: #0078d4; }"
        "QRadioButton::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QCheckBox:disabled { color: %10; }"
        "QCheckBox::indicator:disabled { border-color: #444; background: transparent; }"

        "QCheckBox#rowCheck { spacing: 0; }"
        "QCheckBox#rowCheck::indicator { width: 15px; height: 15px; border: 1px solid #555; border-radius: 8px; background: transparent; }"
        "QCheckBox#rowCheck::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QCheckBox#rowCheck::indicator:hover   { border-color: #0078d4; }"

        "QTableWidget#packageTable { background-color: transparent; border: none; outline: none; gridline-color: transparent; }"
        "QTableWidget#packageTable::item { border: none; border-bottom: 1px solid %8; padding: 4px 6px; background-color: transparent; color: %2; font-size: 13px; }"
        "QTableWidget#packageTable::item:selected { background-color: rgba(0,120,212,0.18); color: %2; }"

        "QHeaderView#pkgHeader::section { background-color: %11; color: %12; font-weight: 600; font-size: 12px; padding: 8px 6px; border: none; border-bottom: 1px solid %13; }"
        "QHeaderView#pkgHeader::section:first { border-top-left-radius: 4px; }"
        "QHeaderView#pkgHeader::section:last  { border-top-right-radius: 4px; }"
        "QHeaderView { background-color: transparent; border: none; }"

        "QTableWidget#packageTable QScrollBar:vertical { background: transparent; width: 6px; margin: 0; }"
        "QTableWidget#packageTable QScrollBar::handle:vertical { background: #444; border-radius: 3px; min-height: 20px; }"
        "QTableWidget#packageTable QScrollBar::add-line:vertical,"
        "QTableWidget#packageTable QScrollBar::sub-line:vertical { height: 0; }"

        "QWidget#outputPanel { background-color: transparent; border-top: 1px solid %8; }"
        "QFrame#separator    { background: %8; max-height: 1px; border: none; }"
        "QFrame#packageArea  { background-color: transparent; border: none; }"

        "QStatusBar { background-color: #007acc; color: #fff; font-size: 12px; }"
        "QDialog { background-color: %3; color: %2; }"

        "QSpinBox#settingsSpin { background-color: %4; border: 1px solid %9; border-radius: 4px; padding: 3px 6px; color: %2; }"
        "QSpinBox#settingsSpin::up-button, QSpinBox#settingsSpin::down-button { background-color: %6; border: none; width: 16px; }"
        "QPushButton#themeBtnLeft  { background-color: %4; color: %10; border: 1px solid %9; border-right: none; border-top-left-radius: 4px; border-bottom-left-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnRight { background-color: %4; color: %10; border: 1px solid %9; border-top-right-radius: 4px; border-bottom-right-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnLeft:checked, QPushButton#themeBtnRight:checked { background-color: #0078d4; color: #fff; border-color: #0078d4; }"

        "QSplitter#mainSplitter::handle { background-color: %8; height: 2px; }"
        "QSplitter#mainSplitter::handle:hover { background-color: #0078d4; }"

        "QPlainTextEdit#terminalView { background-color: %7; color: #8fbcbb; font-family: 'JetBrains Mono', 'Cascadia Code', monospace; font-size: 12px; border: none; padding: 6px; }"

        "QLabel#updateStatusIcon  { color: %10; font-size: 13px; }"
        "QLabel#updateStatusLabel { color: %10; font-size: 13px; }"
        "QLabel#updateEmptyIcon   { font-size: 36px; color: %10; }"
        "QLabel#repoBadge { background-color: %6; color: %2; border-radius: 4px; font-size: 11px; padding: 2px 6px; }"

        "QWidget#updateSection { background-color: %3; }"
        "QLabel#updateDialogTitle { font-size: 15px; font-weight: 600; color: %2; }"
        "QLabel#updateDialogKey   { color: %10; font-size: 12px; }"
        "QLabel#updateDialogValue { color: %2;  font-size: 13px; }"
        "QLabel#updateDialogIcon  { color: %10; font-size: 13px; }"
        "QLabel#updateDialogSectionHdr { color: %2; font-size: 13px; font-weight: 600; }"
        "QTableWidget#updateTable { background-color: %3; border: none; outline: none; gridline-color: transparent; }"
        "QTableWidget#updateTable::item { padding: 6px 8px; color: %2; border: none; border-bottom: 1px solid %8; background-color: transparent; }"
        "QHeaderView#updateTableHeader::section { background-color: %1; color: %10; font-weight: 600; font-size: 12px; padding: 6px 8px; border: none; border-bottom: 1px solid %8; }"
        "QPushButton#updateBtnStart  { background-color: #1a4c6e; color: #a8d8f0; border: none; font-size: 14px; }"
        "QPushButton#updateBtnStart:hover  { background-color: #1e5c84; }"
        "QPushButton#updateBtnCancel { background-color: #1a3a4c; color: #a8d8f0; border: none; font-size: 14px; }"
        "QPushButton#updateBtnCancel:hover { background-color: #1e4a5c; }"

        "QPushButton#maintenanceCardBtn {"
        "  background-color: %4; border: 1px solid %9; border-radius: 6px; text-align: left; padding: 0;"
        "}"
        "QPushButton#maintenanceCardBtn:hover { background-color: %5; border-color: #0078d4; }"
        "QPushButton#maintenanceCardBtn:pressed { background-color: %6; }"
        "QPushButton#maintenanceCardBtn:disabled { border-color: %8; opacity: 0.45; }"
        "QFrame#maintenanceLogFrame { background-color: %4; border: 1px solid %8; border-radius: 6px; }"
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background-color: transparent; }"

        // ── Flatpak page ──────────────────────────────────────────────────────
        "QWidget#flatpakDetailPane { background-color: transparent; }"
        "QWidget#flatpakDetailBlock { background-color: %4; border: 1px solid %8; border-radius: 5px; }"
        "QPushButton#flatpakActionBtn { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; text-align: left; }"
        "QPushButton#flatpakActionBtn:hover { background-color: %5; }"
        "QPushButton#flatpakActionBtn:disabled { color: %10; border-color: %8; }"
        "QPushButton#flatpakUninstallBtn { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; text-align: left; }"
        "QPushButton#flatpakUninstallBtn:hover { background-color: #3a1e1e; border-color: #5a2a2a; color: #d4d4d4; }"
        "QPushButton#flatpakUninstallBtn:disabled { color: %10; border-color: %8; }"

        // ── Repository page ───────────────────────────────────────────────────
        // Detail pane header bar
        "QWidget#repoDetailHeader { background-color: %3; border-bottom: 1px solid %8; }"

        // Footer Add / Edit / Remove buttons — full-width, joined
        "QPushButton#repoFooterBtnAdd {"
        "  background-color: %4; color: %2;"
        "  border: 1px solid %9;"
        "  border-right: none;"
        "  border-bottom-left-radius: 5px;"
        "  border-top-left-radius: 5px;"
        "  padding: 0 14px;"
        "}"
        "QPushButton#repoFooterBtnAdd:hover { background-color: %5; }"

        "QPushButton#repoFooterBtnEdit {"
        "  background-color: %4; color: %2;"
        "  border: 1px solid %9;"
        "  border-radius: 0;"
        "  padding: 0 14px;"
        "}"
        "QPushButton#repoFooterBtnEdit:hover { background-color: %5; }"
        "QPushButton#repoFooterBtnEdit:disabled { color: %10; }"

        "QPushButton#repoFooterBtnRemove {"
        "  background-color: %4; color: %2;"
        "  border: 1px solid %9;"
        "  border-left: none;"
        "  border-bottom-right-radius: 5px;"
        "  border-top-right-radius: 5px;"
        "  padding: 0 14px;"
        "}"
        "QPushButton#repoFooterBtnRemove:hover { background-color: #3a1e1e; border-color: #5a2a2a; color: #d4d4d4; }"
        "QPushButton#repoFooterBtnRemove:disabled { color: %10; }"

        // ── Kernel page ───────────────────────────────────────────────────────
        "QPushButton#kernelFooterBtnInstall {"
        "  background-color: #1e3a52; color: #a8d8f0;"
        "  border: none;"
        "  border-right: 1px solid rgba(0,120,212,0.3);"
        "  font-size: 13px;"
        "}"
        "QPushButton#kernelFooterBtnInstall:hover { background-color: #1e4c6e; }"
        "QPushButton#kernelFooterBtnInstall:disabled { opacity: 0.5; }"
        "QPushButton#kernelFooterBtnRemove {"
        "  background-color: #1a2a3a; color: #a8d8f0;"
        "  border: none;"
        "  font-size: 13px;"
        "}"
        "QPushButton#kernelFooterBtnRemove:hover { background-color: #3a1e1e; color: #d4d4d4; }"
        "QPushButton#kernelFooterBtnRemove:disabled { opacity: 0.5; }";

    setStyleSheet(qss
        .arg(bg,          // %1
             textPrimary, // %2
             bgNav,       // %3
             bgWidget,    // %4
             bgHover,     // %5
             bgChecked,   // %6
             bgTerminal,  // %7
             border,      // %8
             borderInput, // %9
             textMuted)   // %10
        .arg(headerBg,     // %11
             headerFg,     // %12
             headerBorder) // %13
    );
}