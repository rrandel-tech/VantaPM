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

    statusBar()->showMessage("Found 1789 packages");
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
    navBar->setFixedHeight(46);

    auto *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(8, 4, 8, 4);
    navLayout->setSpacing(4);

    auto makeNavBtn = [](const QString &label, bool active = false) {
        auto *btn = new QPushButton(label);
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setChecked(active);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    m_btnSearch      = makeNavBtn("  Search", true);
    m_btnInstalled   = makeNavBtn("  Installed");
    m_btnSysUpdate   = makeNavBtn("  System Update");
    m_btnMaintenance = makeNavBtn("  Maintenance");
    m_btnFlatpak     = makeNavBtn("  Flatpak");
    m_btnRepository  = makeNavBtn("  Repository");
    m_btnKernel      = makeNavBtn("  Kernel");

    for (auto *btn : {m_btnSearch, m_btnInstalled, m_btnSysUpdate,
                      m_btnMaintenance, m_btnFlatpak, m_btnRepository, m_btnKernel})
        navLayout->addWidget(btn);

    navLayout->addStretch();

    auto *btnRefresh  = new QPushButton("⟳");
    auto *btnSettings = new QPushButton("⚙");
    btnRefresh->setObjectName("iconBtn");
    btnSettings->setObjectName("iconBtn");
    btnRefresh->setFixedSize(32, 32);
    btnSettings->setFixedSize(32, 32);
    navLayout->addWidget(btnRefresh);
    navLayout->addWidget(btnSettings);

    m_settingsDialog = new SettingsDialog(this);
    connect(btnSettings, &QPushButton::clicked, this, [this]() {
        m_settingsDialog->exec();
    });

    rootLayout->addWidget(navBar);

    // ── Page stack ────────────────────────────────────────────────────────────
    m_pageStack = new QStackedWidget;

    m_searchPage = new SearchPage;

    m_pageStack->addWidget(m_searchPage);   // index 0 — Search

    // Placeholder pages for the remaining nav items (stubs)
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

    // ── Nav button → page switching ───────────────────────────────────────────
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

void MainWindow::applyStyleSheet()
{
    qApp->setStyle("Fusion");

    const QString qss = R"(
QMainWindow, QWidget {
    background-color: #1e1e1e;
    color: #d4d4d4;
    font-family: "Segoe UI", "Noto Sans", sans-serif;
    font-size: 13px;
}

#navBar {
    background-color: #252526;
    border-bottom: 1px solid #333;
}

QPushButton#navBtn {
    background-color: transparent;
    color: #aaa;
    border: none;
    border-radius: 5px;
    padding: 5px 12px;
    font-size: 13px;
    text-align: left;
}
QPushButton#navBtn:hover   { background-color: #2d2d2d; color: #d4d4d4; }
QPushButton#navBtn:checked { background-color: #37373d; color: #ffffff; }

QPushButton#iconBtn {
    background-color: transparent;
    border: none;
    color: #888;
    font-size: 16px;
    border-radius: 4px;
}
QPushButton#iconBtn:hover { color: #d4d4d4; background-color: #2d2d2d; }

#contentWrapper { background-color: #1e1e1e; }

QLabel#pageTitle { font-size: 16px; font-weight: 600; color: #d4d4d4; }

QLineEdit#searchInput {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-radius: 5px;
    padding: 6px 10px;
    color: #d4d4d4;
}
QLineEdit#searchInput:focus { border: 1px solid #0078d4; }

QPushButton#btnPrimary {
    background-color: #0078d4;
    color: #fff;
    border: none;
    border-radius: 5px;
    padding: 0 16px;
}
QPushButton#btnPrimary:hover   { background-color: #1a8de0; }
QPushButton#btnPrimary:pressed { background-color: #006cbf; }

QPushButton#btnSecondary {
    background-color: #37373d;
    color: #d4d4d4;
    border: 1px solid #444;
    border-radius: 5px;
    padding: 0 14px;
}
QPushButton#btnSecondary:hover { background-color: #3f3f46; }

QPushButton#btnInstall {
    background-color: #1e6b3a;
    color: #d4d4d4;
    border: none;
    border-radius: 5px;
    padding: 0 14px;
}
QPushButton#btnInstall:hover { background-color: #237a43; }

QPushButton#btnRemove {
    background-color: #2d2d2d;
    color: #d4d4d4;
    border: 1px solid #444;
    border-radius: 5px;
    padding: 0 14px;
}
QPushButton#btnRemove:hover { background-color: #3a1e1e; }

QLabel#sectionLabel { color: #aaa; font-size: 12px; }
QLabel#filterLabel  { color: #aaa; }

QComboBox#filterCombo {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    padding: 4px 8px;
    color: #d4d4d4;
    min-width: 130px;
}
QComboBox#filterCombo::drop-down { border: none; }
QComboBox#filterCombo QAbstractItemView {
    background-color: #252526;
    border: 1px solid #3c3c3c;
    color: #d4d4d4;
    selection-background-color: #0078d4;
}

QCheckBox, QRadioButton { color: #d4d4d4; spacing: 6px; }
QCheckBox::indicator, QRadioButton::indicator {
    width: 14px; height: 14px;
    border: 1px solid #555;
    border-radius: 3px;
    background: #2d2d2d;
}
QCheckBox::indicator:checked { background: #0078d4; border-color: #0078d4; }
QRadioButton::indicator { border-radius: 7px; }
QRadioButton::indicator:checked { background: #0078d4; border-color: #0078d4; }

QFrame#separator { color: #333; background: #333; max-height: 1px; border: none; }

QFrame#packageArea {
    background-color: #252526;
    border: 1px solid #333;
    border-radius: 6px;
}

QLabel#emptyIcon  { font-size: 36px; color: #555; }
QLabel#emptyLabel { color: #888; font-size: 14px; margin-top: 6px; }
QLabel#emptyHint  { color: #555; font-size: 12px; }

QTextEdit#terminalOutput {
    background-color: #1a1a1a;
    border: 1px solid #333;
    border-radius: 5px;
    color: #8fbcbb;
    font-family: "JetBrains Mono", "Cascadia Code", "Fira Mono", monospace;
    font-size: 12px;
    padding: 6px;
}

QStatusBar { background-color: #007acc; color: #fff; font-size: 12px; }

/* Settings dialog */
QDialog {
    background-color: #252526;
    color: #d4d4d4;
}

QSpinBox#settingsSpin {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    padding: 3px 6px;
    color: #d4d4d4;
}
QSpinBox#settingsSpin::up-button, QSpinBox#settingsSpin::down-button {
    background-color: #37373d;
    border: none;
    width: 16px;
}

QPushButton#themeBtnLeft {
    background-color: #37373d;
    color: #aaa;
    border: 1px solid #444;
    border-right: none;
    border-radius: 0;
    border-top-left-radius: 4px;
    border-bottom-left-radius: 4px;
    padding: 0 14px;
}
QPushButton#themeBtnRight {
    background-color: #37373d;
    color: #aaa;
    border: 1px solid #444;
    border-radius: 0;
    border-top-right-radius: 4px;
    border-bottom-right-radius: 4px;
    padding: 0 14px;
}
QPushButton#themeBtnLeft:checked,
QPushButton#themeBtnRight:checked {
    background-color: #0078d4;
    color: #fff;
    border-color: #0078d4;
}

QSplitter#mainSplitter::handle {
    background-color: #333;
    height: 3px;
}
QSplitter#mainSplitter::handle:hover {
    background-color: #0078d4;
}

QTabWidget#terminalTabs::pane {
    border: 1px solid #333;
    border-radius: 0 0 5px 5px;
    background-color: #1a1a1a;
}
QTabWidget#terminalTabs > QTabBar::tab {
    background-color: #252526;
    color: #888;
    border: 1px solid #333;
    border-bottom: none;
    padding: 4px 14px;
    font-size: 12px;
}
QTabWidget#terminalTabs > QTabBar::tab:selected {
    background-color: #1a1a1a;
    color: #d4d4d4;
    border-bottom: 1px solid #1a1a1a;
}
QTabWidget#terminalTabs > QTabBar::tab:hover:!selected {
    background-color: #2d2d2d;
    color: #ccc;
}

QPlainTextEdit#terminalView {
    background-color: #1a1a1a;
    color: #8fbcbb;
    font-family: "JetBrains Mono", "Cascadia Code", "Fira Mono", monospace;
    font-size: 12px;
    border: none;
    padding: 4px;
}

)";

    setStyleSheet(qss);
}