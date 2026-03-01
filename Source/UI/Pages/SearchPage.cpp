#include "SearchPage.hpp"
#include "UI/Widgets/TerminalWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QScrollBar>

SearchPage::SearchPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SearchPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(12);

    // ── Top controls ──────────────────────────────────────────────────────────
    auto *topSection = new QWidget;
    topSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *topLayout = new QVBoxLayout(topSection);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    // Title
    auto *titleRow = new QHBoxLayout;
    auto *titleIcon = new QLabel;
    titleIcon->setPixmap(QPixmap(":/icons/light/search.png")
                             .scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *pageTitle = new QLabel("Search Packages");
    pageTitle->setObjectName("pageTitle");
    titleRow->addWidget(titleIcon);
    titleRow->addWidget(pageTitle);
    titleRow->addStretch();
    topLayout->addLayout(titleRow);

    // Search bar
    auto *searchRow = new QHBoxLayout;
    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText("Search packages...");
    m_searchInput->setObjectName("searchInput");
    m_searchInput->setFixedHeight(36);

    m_btnSearch = new QPushButton("Search");
    m_btnSearch->setObjectName("btnPrimary");
    m_btnSearch->setIconSize(QSize(15, 15));
    m_btnSearch->setFixedHeight(36);
    m_btnSearch->setMinimumWidth(95);

    m_btnClear = new QPushButton("Clear");
    m_btnClear->setObjectName("btnSecondary");
    m_btnClear->setIconSize(QSize(15, 15));
    m_btnClear->setFixedHeight(36);
    m_btnClear->setMinimumWidth(80);

    searchRow->addWidget(m_searchInput);
    searchRow->addWidget(m_btnSearch);
    searchRow->addWidget(m_btnClear);
    topLayout->addLayout(searchRow);

    // Separator
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("separator");
    topLayout->addWidget(sep);

    // Filters
    auto *filtersLabel = new QLabel("Filters");
    filtersLabel->setObjectName("sectionLabel");
    topLayout->addWidget(filtersLabel);

    auto *filterRow = new QHBoxLayout;
    filterRow->setSpacing(10);

    auto *repoLabel = new QLabel("Repository:");
    repoLabel->setObjectName("filterLabel");
    m_repoFilter = new QComboBox;
    m_repoFilter->addItems({"All Repositories", "core", "extra", "community", "multilib"});
    m_repoFilter->setObjectName("filterCombo");

    auto *statusLabel = new QLabel("Status:");
    statusLabel->setObjectName("filterLabel");
    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"All Packages", "Installed", "Not Installed", "Upgradable"});
    m_statusFilter->setObjectName("filterCombo");

    m_includeAur    = new QCheckBox("Include AUR");
    m_installedOnly = new QRadioButton("Installed Only");
    m_includeAur->setChecked(true);

    filterRow->addWidget(repoLabel);
    filterRow->addWidget(m_repoFilter);
    filterRow->addSpacing(8);
    filterRow->addWidget(statusLabel);
    filterRow->addWidget(m_statusFilter);
    filterRow->addSpacing(8);
    filterRow->addWidget(m_includeAur);
    filterRow->addWidget(m_installedOnly);
    filterRow->addStretch();
    topLayout->addLayout(filterRow);

    // Action buttons
    auto *actionRow = new QHBoxLayout;

    m_btnInstall = new QPushButton("Install Selected");
    m_btnInstall->setObjectName("btnInstall");
    m_btnInstall->setIconSize(QSize(15, 15));
    m_btnInstall->setFixedHeight(34);

    m_btnRemove = new QPushButton("Remove Selected");
    m_btnRemove->setObjectName("btnRemove");
    m_btnRemove->setIconSize(QSize(15, 15));
    m_btnRemove->setFixedHeight(34);

    m_btnSelAll = new QPushButton("Select All");
    m_btnSelAll->setObjectName("btnSecondary");
    m_btnSelAll->setFixedHeight(34);

    m_btnClrSel = new QPushButton("Clear Selection");
    m_btnClrSel->setObjectName("btnSecondary");
    m_btnClrSel->setIconSize(QSize(15, 15));
    m_btnClrSel->setFixedHeight(34);

    actionRow->addWidget(m_btnInstall);
    actionRow->addWidget(m_btnRemove);
    actionRow->addStretch();
    actionRow->addWidget(m_btnSelAll);
    actionRow->addWidget(m_btnClrSel);
    topLayout->addLayout(actionRow);

    rootLayout->addWidget(topSection);

    // ── Splitter ──────────────────────────────────────────────────────────────
    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("mainSplitter");
    splitter->setChildrenCollapsible(false);

    auto *packageArea = new QFrame;
    packageArea->setObjectName("packageArea");
    packageArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *pkgLayout = new QVBoxLayout(packageArea);
    pkgLayout->setAlignment(Qt::AlignCenter);
    pkgLayout->setSpacing(6);

    m_emptyIcon = new QLabel;
    m_emptyIcon->setPixmap(QPixmap(":/icons/light/empty.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_emptyIcon->setAlignment(Qt::AlignCenter);

    m_emptyLabel = new QLabel("No packages found");
    m_emptyLabel->setObjectName("emptyLabel");
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    auto *emptyHint = new QLabel("Try adjusting your search criteria or filters");
    emptyHint->setObjectName("emptyHint");
    emptyHint->setAlignment(Qt::AlignCenter);

    pkgLayout->addWidget(m_emptyIcon);
    pkgLayout->addWidget(m_emptyLabel);
    pkgLayout->addWidget(emptyHint);
    splitter->addWidget(packageArea);

    // Terminal tabs
    m_bottomTabs = new QTabWidget;
    m_bottomTabs->setObjectName("terminalTabs");
    m_bottomTabs->setIconSize(QSize(14, 14));
    m_bottomTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_terminal = new TerminalWidget;
    m_terminal->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bottomTabs->addTab(m_terminal, "Terminal");

    m_outputView = new QPlainTextEdit;
    m_outputView->setObjectName("terminalView");
    m_outputView->setReadOnly(true);
    m_outputView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outputView->setPlaceholderText("Operation output will appear here...");
    m_bottomTabs->addTab(m_outputView, "Output");

    splitter->addWidget(m_bottomTabs);
    splitter->setSizes({450, 220});

    rootLayout->addWidget(splitter, 1);
}

void SearchPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";

    m_btnSearch->setIcon(QIcon(prefix + "search.png"));
    m_btnClear->setIcon(QIcon(prefix + "clear.png"));
    m_btnInstall->setIcon(QIcon(prefix + "install.png"));
    m_btnRemove->setIcon(QIcon(prefix + "remove.png"));
    m_btnClrSel->setIcon(QIcon(prefix + "clear.png"));

    m_bottomTabs->setTabIcon(0, QIcon(prefix + "terminal.png"));
    m_bottomTabs->setTabIcon(1, QIcon(prefix + "output.png"));

    m_emptyIcon->setPixmap(QPixmap(prefix + "empty.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void SearchPage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}