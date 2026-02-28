#include "MainWindow.hpp"
#include "UI/Pages/SearchPage.hpp"
#include "UI/SettingsDialog.hpp"

#include <QApplication>
#include <QStatusBar>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("VantaPM");
    setMinimumSize(1100, 700);
    resize(1456, 816);

    setupUi();
    applyStyleSheet();
    updateIcons();

    statusBar()->showMessage("  Found 1789 packages");
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

    connect(m_btnTheme, &QPushButton::clicked, this, &MainWindow::toggleTheme);

    m_settingsDialog = new SettingsDialog(this);
    connect(m_btnSettings, &QPushButton::clicked, this, [this]() {
        m_settingsDialog->exec();
    });

    rootLayout->addWidget(navBar);

    // ── Page stack ────────────────────────────────────────────────────────────
    m_pageStack = new QStackedWidget;
    m_searchPage = new SearchPage;
    m_pageStack->addWidget(m_searchPage);

    for (int i = 1; i <= 6; ++i) {
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

    for (int i = 0; i < navBtns.size(); ++i) {
        connect(navBtns[i], &QPushButton::clicked, this, [this, navBtns, i]() {
            m_pageStack->setCurrentIndex(i);
            for (int j = 0; j < navBtns.size(); ++j)
                navBtns[j]->setChecked(j == i);
        });
    }
}

void MainWindow::toggleTheme()
{
    m_isDark = !m_isDark;
    applyStyleSheet();
    updateIcons();
    m_searchPage->updateIcons(m_isDark);
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
    qApp->setStyle("Fusion");

    const QString bg         = m_isDark ? "#1e1e1e" : "#f5f5f5";
    const QString bgNav      = m_isDark ? "#252526" : "#e8e8e8";
    const QString bgWidget   = m_isDark ? "#2a2a2a" : "#ffffff";
    const QString bgHover    = m_isDark ? "#2a2a2a" : "#e0e0e0";
    const QString bgChecked  = m_isDark ? "#37373d" : "#d6d6d6";
    const QString bgTerminal = m_isDark ? "#1a1a1a" : "#ffffff";
    const QString border     = m_isDark ? "#2d2d2d" : "#d0d0d0";
    const QString borderInput= m_isDark ? "#383838" : "#c0c0c0";
    const QString textPrimary= m_isDark ? "#d4d4d4" : "#1e1e1e";
    const QString textMuted  = m_isDark ? "#999"    : "#666";

    const QString qss =
        "QMainWindow, QWidget { background-color: " + bg + "; color: " + textPrimary + "; font-family: 'Noto Sans', 'Segoe UI', sans-serif; font-size: 13px; }"
        "#navBar { background-color: " + bgNav + "; border-bottom: 1px solid " + border + "; }"
        "QPushButton#navBtn { background-color: transparent; color: " + textMuted + "; border: none; border-radius: 5px; padding: 5px 14px; font-size: 13px; text-align: center; }"
        "QPushButton#navBtn:hover { background-color: " + bgHover + "; color: " + textPrimary + "; }"
        "QPushButton#navBtn:checked { background-color: " + bgChecked + "; color: " + textPrimary + "; border-bottom: 2px solid #0078d4; }"
        "QPushButton#iconBtn { background-color: transparent; border: none; border-radius: 5px; }"
        "QPushButton#iconBtn:hover { background-color: " + bgHover + "; }"
        "#contentWrapper { background-color: " + bg + "; }"
        "QLabel#pageTitle { font-size: 15px; font-weight: 600; color: " + textPrimary + "; }"
        "QLineEdit#searchInput { background-color: " + bgWidget + "; border: 1px solid " + borderInput + "; border-radius: 5px; padding: 6px 10px; color: " + textPrimary + "; }"
        "QLineEdit#searchInput:focus { border-color: #0078d4; }"
        "QPushButton#btnPrimary { background-color: #0078d4; color: #fff; border: none; border-radius: 5px; padding: 0 16px; text-align: center; }"
        "QPushButton#btnPrimary:hover { background-color: #1a8de0; }"
        "QPushButton#btnPrimary:pressed { background-color: #006cbf; }"
        "QPushButton#btnSecondary { background-color: " + bgWidget + "; color: " + textPrimary + "; border: 1px solid " + borderInput + "; border-radius: 5px; padding: 0 14px; text-align: center; }"
        "QPushButton#btnSecondary:hover { background-color: " + bgHover + "; }"
        "QPushButton#btnInstall { background-color: #1e6b3a; color: #d4d4d4; border: none; border-radius: 5px; padding: 0 16px; text-align: center; }"
        "QPushButton#btnInstall:hover { background-color: #237a43; }"
        "QPushButton#btnRemove { background-color: " + bgWidget + "; color: " + textPrimary + "; border: 1px solid " + borderInput + "; border-radius: 5px; padding: 0 14px; text-align: center; }"
        "QPushButton#btnRemove:hover { background-color: #3a1e1e; border-color: #5a2a2a; color: #d4d4d4; }"
        "QLabel#sectionLabel { color: " + textMuted + "; font-size: 12px; font-weight: 600; }"
        "QLabel#filterLabel { color: " + textMuted + "; }"
        "QComboBox#filterCombo { background-color: " + bgWidget + "; border: 1px solid " + borderInput + "; border-radius: 4px; padding: 4px 8px; color: " + textPrimary + "; min-width: 140px; }"
        "QComboBox#filterCombo::drop-down { border: none; width: 20px; }"
        "QComboBox#filterCombo QAbstractItemView { background-color: " + bgNav + "; border: 1px solid " + borderInput + "; color: " + textPrimary + "; selection-background-color: #0078d4; }"
        "QCheckBox, QRadioButton { color: " + textPrimary + "; spacing: 6px; }"
        "QCheckBox::indicator, QRadioButton::indicator { width: 15px; height: 15px; border: 1px solid " + textMuted + "; border-radius: 3px; background: " + bgWidget + "; }"
        "QCheckBox::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QRadioButton::indicator { border-radius: 7px; }"
        "QRadioButton::indicator:checked { background: #0078d4; border-color: #0078d4; }"
        "QFrame#separator { background: " + border + "; max-height: 1px; border: none; }"
        "QFrame#packageArea { background-color: transparent; border: none; }"
        "QLabel#emptyLabel { color: " + textPrimary + "; font-size: 14px; margin-top: 4px; }"
        "QLabel#emptyHint { color: " + textPrimary + "; font-size: 12px; }"
        "QStatusBar { background-color: #007acc; color: #fff; font-size: 12px; }"
        "QDialog { background-color: " + bgNav + "; color: " + textPrimary + "; }"
        "QSpinBox#settingsSpin { background-color: " + bgWidget + "; border: 1px solid " + borderInput + "; border-radius: 4px; padding: 3px 6px; color: " + textPrimary + "; }"
        "QSpinBox#settingsSpin::up-button, QSpinBox#settingsSpin::down-button { background-color: " + bgChecked + "; border: none; width: 16px; }"
        "QPushButton#themeBtnLeft { background-color: " + bgWidget + "; color: " + textMuted + "; border: 1px solid " + borderInput + "; border-right: none; border-top-left-radius: 4px; border-bottom-left-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnRight { background-color: " + bgWidget + "; color: " + textMuted + "; border: 1px solid " + borderInput + "; border-top-right-radius: 4px; border-bottom-right-radius: 4px; padding: 0 14px; }"
        "QPushButton#themeBtnLeft:checked, QPushButton#themeBtnRight:checked { background-color: #0078d4; color: #fff; border-color: #0078d4; }"
        "QSplitter#mainSplitter::handle { background-color: " + border + "; height: 2px; }"
        "QSplitter#mainSplitter::handle:hover { background-color: #0078d4; }"
        "QTabWidget#terminalTabs::pane { border: 1px solid " + border + "; border-radius: 0 0 5px 5px; background-color: " + bgTerminal + "; }"
        "QTabWidget#terminalTabs > QTabBar::tab { background-color: " + bgNav + "; color: " + textMuted + "; border: 1px solid " + border + "; border-bottom: none; padding: 5px 16px; font-size: 12px; }"
        "QTabWidget#terminalTabs > QTabBar::tab:selected { background-color: " + bgTerminal + "; color: " + textPrimary + "; }"
        "QTabWidget#terminalTabs > QTabBar::tab:hover:!selected { background-color: " + bgHover + "; color: " + textPrimary + "; }"
        "QPlainTextEdit#terminalView { background-color: " + bgTerminal + "; color: #8fbcbb; font-family: 'JetBrains Mono', 'Cascadia Code', monospace; font-size: 12px; border: none; padding: 6px; }";

    setStyleSheet(qss);
}