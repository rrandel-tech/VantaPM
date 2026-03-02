#include "MainWindow.hpp"
#include "UI/Pages/SearchPage.hpp"
#include "UI/Pages/InstalledPage.hpp"
#include "UI/Pages/UpdatePage.hpp"
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

    // Honour persisted theme before first paint
    m_isDark = (SettingsManager::instance().theme() == "dark");

    setupUi();
    applyStyleSheet();
    updateIcons();

    statusBar()->showMessage("  Ready");

    // Status bar — each page reports its own message
    connect(m_searchPage,   &SearchPage::statusMessage,
            this, [this](const QString &msg) { statusBar()->showMessage(msg); });
    connect(m_installedPage, &InstalledPage::statusMessage,
            this, [this](const QString &msg) {
        // Only update the status bar if Installed is the currently visible page
        if (m_pageStack->currentIndex() == 1)
            statusBar()->showMessage(msg);
    });
    connect(m_updatePage,   &UpdatePage::statusMessage,
            this, [this](const QString &msg) { statusBar()->showMessage(msg); });

    // When the settings dialog switches theme, apply it live
    connect(m_settingsDialog, &SettingsDialog::themeChanged,
            this, [this](const QString &theme) {
        m_isDark = (theme == "dark");
        applyStyleSheet();
        updateIcons();
        m_searchPage->updateIcons(m_isDark);
        m_installedPage->updateIcons(m_isDark);
    });

    // Auto-refresh: re-query the installed list on each timer tick
    connect(&SettingsManager::instance(), &SettingsManager::autoRefreshTriggered,
            this, [this]() {
        m_installedPage->loadPackages();
        SettingsManager::instance().notify(
            QStringLiteral("VantaPM"),
            QStringLiteral("Package list refreshed"));
    });

    // Defer the initial package load until after the event loop starts
    // so the window is fully painted before pacman is invoked.
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

    // Construct dialog early so theme button can reference it
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
    m_pageStack->addWidget(m_searchPage);          // index 0

    m_installedPage = new InstalledPage;
    m_pageStack->addWidget(m_installedPage);        // index 1

    m_updatePage = new UpdatePage;
    m_pageStack->addWidget(m_updatePage);           // index 2

    for (int i = 3; i <= 6; ++i) {
        auto *stub = new QWidget;
        auto *l    = new QVBoxLayout(stub);
        auto *lbl  = new QLabel("Coming soon");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setObjectName("emptyLabel");
        l->addWidget(lbl);
        m_pageStack->addWidget(stub);
    }

    auto *contentWrapper = new QWidget;
    contentWrapper->setObjectName("contentWrapper");
    auto *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->addWidget(m_pageStack);
    rootLayout->addWidget(contentWrapper, 1);

    const QList<QPushButton *> navBtns = {
        m_btnSearch, m_btnInstalled, m_btnSysUpdate,
        m_btnMaintenance, m_btnFlatpak, m_btnRepository, m_btnKernel
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

            // For pages with live counts, show the real count if available
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
    // Set Fusion style once only — re-calling setStyle() on every theme switch
    // reinstantiates the style engine across the entire widget tree.
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

    // IMPORTANT: do NOT use "QWidget { ... }" as a base rule.
    // That selector matches every cell-widget checkbox and button wrapper in the
    // table (~2800 widgets) and forces Qt to re-style each one individually on
    // every theme switch, which hangs the UI.
    // Instead, scope all background/color rules to named object IDs or to
    // specific widget subclasses that don't appear as anonymous cell wrappers.
    const QString qss =
        // ── Named containers only for base colours ───────────────────────────
        "QMainWindow { background-color: %1; color: %2; font-family: 'Noto Sans', 'Segoe UI', sans-serif; font-size: 13px; }"
        "#contentWrapper { background-color: %1; color: %2; font-family: 'Noto Sans', 'Segoe UI', sans-serif; font-size: 13px; }"
        "#navBar { background-color: %3; border-bottom: 1px solid %8; }"

        // ── Nav buttons ──────────────────────────────────────────────────────
        "QPushButton#navBtn { background-color: transparent; color: %10; border: none; border-radius: 5px; padding: 5px 14px; font-size: 13px; }"
        "QPushButton#navBtn:hover { background-color: %5; color: %2; }"
        "QPushButton#navBtn:checked { background-color: %6; color: %2; border-bottom: 2px solid #0078d4; }"
        "QPushButton#iconBtn { background-color: transparent; border: none; border-radius: 5px; }"
        "QPushButton#iconBtn:hover { background-color: %5; }"

        // ── Page title ───────────────────────────────────────────────────────
        "QLabel#pageTitle { font-size: 15px; font-weight: 600; color: %2; }"

        // ── Search input ─────────────────────────────────────────────────────
        "QLineEdit#searchInput { background-color: %4; border: 1px solid %9; border-radius: 5px; padding: 6px 10px; color: %2; }"
        "QLineEdit#searchInput:focus { border-color: #0078d4; }"

        // ── Action buttons ───────────────────────────────────────────────────
        "QPushButton#btnPrimary { background-color: #0078d4; color: #fff; border: none; border-radius: 5px; padding: 0 16px; }"
        "QPushButton#btnPrimary:hover { background-color: #1a8de0; }"
        "QPushButton#btnPrimary:pressed { background-color: #006cbf; }"
        "QPushButton#btnSecondary { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; }"
        "QPushButton#btnSecondary:hover { background-color: %5; }"
        "QPushButton#btnInstall { background-color: #1e6b3a; color: #d4d4d4; border: none; border-radius: 5px; padding: 0 16px; }"
        "QPushButton#btnInstall:hover { background-color: #237a43; }"
        "QPushButton#btnRemove { background-color: %4; color: %2; border: 1px solid %9; border-radius: 5px; padding: 0 14px; }"
        "QPushButton#btnRemove:hover { background-color: #3a1e1e; border-color: #5a2a2a; color: #d4d4d4; }"

        // ── Labels ───────────────────────────────────────────────────────────
        "QLabel#sectionLabel { color: %10; font-size: 12px; font-weight: 600; }"
        "QLabel#filterLabel  { color: %10; }"
        "QLabel#pkgName      { font-size: 13px; color: %2; }"
        "QLabel#pkgVersion   { color: %10; font-size: 12px; }"
        "QLabel#pkgDesc      { color: %10; font-size: 12px; }"
        "QLabel#emptyLabel   { color: %2; font-size: 14px; margin-top: 4px; }"
        "QLabel#emptyHint    { color: %2; font-size: 12px; }"
        "QLabel#statusInstalled    { color: #4ec994; font-size: 12px; }"
        "QLabel#statusNotInstalled { color: %10; font-size: 12px; }"

        // ── Combo boxes ──────────────────────────────────────────────────────
        "QComboBox#filterCombo { background-color: %4; border: 1px solid %9; border-radius: 4px; padding: 4px 8px; color: %2; min-width: 140px; }"
        "QComboBox#filterCombo::drop-down { border: none; width: 20px; }"
        "QComboBox#filterCombo QAbstractItemView { background-color: %3; border: 1px solid %9; color: %2; selection-background-color: #0078d4; }"

        // ── Checkboxes / radios ──────────────────────────────────────────────
        "QCheckBox  { color: %2; spacing: 6px; }"
        "QRadioButton { color: %2; spacing: 6px; }"
        "QCheckBox::indicator  { width: 15px; height: 15px; border: 1px solid %10; border-radius: 3px; background: %4; }"
        "QRadioButton::indicator { width: 15px; height: 15px; border: 1px solid %10; border-radius: 7px; background: %4; }"
        "QCheckBox::indicator:checked   { background: #0078d4; border-color: #0078d4; }"
        "QRadioButton::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QCheckBox:disabled { color: %10; }"
        "QCheckBox::indicator:disabled { border-color: #444; background: transparent; }"

        // ── Row checkboxes (circular, inside table cell widgets) ──────────────
        "QCheckBox#rowCheck { spacing: 0; }"
        "QCheckBox#rowCheck::indicator { width: 15px; height: 15px; border: 1px solid #555; border-radius: 8px; background: transparent; }"
        "QCheckBox#rowCheck::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QCheckBox#rowCheck::indicator:hover   { border-color: #0078d4; }"

        // ── Package table ────────────────────────────────────────────────────
        "QTableWidget#packageTable { background-color: transparent; border: none; outline: none; gridline-color: transparent; }"
        "QTableWidget#packageTable::item { border: none; border-bottom: 1px solid %8; padding: 4px 6px; background-color: transparent; color: %2; font-size: 13px; }"
        "QTableWidget#packageTable::item:selected { background-color: transparent; color: %2; }"

        // ── Table header ─────────────────────────────────────────────────────
        "QHeaderView#pkgHeader::section { background-color: %11; color: %12; font-weight: 600; font-size: 12px; padding: 8px 6px; border: none; border-bottom: 1px solid %13; }"
        "QHeaderView#pkgHeader::section:first { border-top-left-radius: 4px; }"
        "QHeaderView#pkgHeader::section:last  { border-top-right-radius: 4px; }"
        "QHeaderView { background-color: transparent; border: none; }"

        // ── Scrollbar ────────────────────────────────────────────────────────
        "QTableWidget#packageTable QScrollBar:vertical { background: transparent; width: 6px; margin: 0; }"
        "QTableWidget#packageTable QScrollBar::handle:vertical { background: #444; border-radius: 3px; min-height: 20px; }"
        "QTableWidget#packageTable QScrollBar::add-line:vertical,"
        "QTableWidget#packageTable QScrollBar::sub-line:vertical { height: 0; }"

        // ── Output / panels ──────────────────────────────────────────────────
        "QWidget#outputPanel { background-color: transparent; border-top: 1px solid %8; }"
        "QFrame#separator    { background: %8; max-height: 1px; border: none; }"
        "QFrame#packageArea  { background-color: transparent; border: none; }"

        // ── Status bar / dialog ──────────────────────────────────────────────
        "QStatusBar { background-color: #007acc; color: #fff; font-size: 12px; }"
        "QDialog { background-color: %3; color: %2; }"

        // ── Settings dialog ──────────────────────────────────────────────────
        "QSpinBox#settingsSpin { background-color: %4; border: 1px solid %9; border-radius: 4px; padding: 3px 6px; color: %2; }"
        "QSpinBox#settingsSpin::up-button, QSpinBox#settingsSpin::down-button { background-color: %6; border: none; width: 16px; }"
        "QPushButton#themeBtnLeft  { background-color: %4; color: %10; border: 1px solid %9; border-right: none; border-top-left-radius: 4px; border-bottom-left-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnRight { background-color: %4; color: %10; border: 1px solid %9; border-top-right-radius: 4px; border-bottom-right-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnLeft:checked, QPushButton#themeBtnRight:checked { background-color: #0078d4; color: #fff; border-color: #0078d4; }"

        // ── Splitter ─────────────────────────────────────────────────────────
        "QSplitter#mainSplitter::handle { background-color: %8; height: 2px; }"
        "QSplitter#mainSplitter::handle:hover { background-color: #0078d4; }"

        // ── Terminal output view ──────────────────────────────────────────────
        "QPlainTextEdit#terminalView { background-color: %7; color: #8fbcbb; font-family: 'JetBrains Mono', 'Cascadia Code', monospace; font-size: 12px; border: none; padding: 6px; }"

        // ── Update page ───────────────────────────────────────────────────────
        "QLabel#updateStatusIcon  { color: %10; font-size: 13px; }"
        "QLabel#updateStatusLabel { color: %10; font-size: 13px; }"
        "QLabel#updateEmptyIcon   { font-size: 36px; color: %10; }"
        "QLabel#repoBadge { background-color: %6; color: %2; border-radius: 4px; font-size: 11px; padding: 2px 6px; }"

        // ── Update confirm dialog ─────────────────────────────────────────────
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
        "QPushButton#updateBtnCancel:hover { background-color: #1e4a5c; }";

    // Apply on the MainWindow only, not qApp — avoids cascading into
    // every anonymous cell widget in QTableWidget viewports.
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