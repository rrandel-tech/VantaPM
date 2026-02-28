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

    // ── Fixed top controls ────────────────────────────────────────────────────
    auto *topSection = new QWidget;
    topSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *topLayout = new QVBoxLayout(topSection);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(12);

    // Page title
    auto *pageTitle = new QLabel("  Search Packages");
    pageTitle->setObjectName("pageTitle");
    topLayout->addWidget(pageTitle);

    // Search bar
    auto *searchRow = new QHBoxLayout;
    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText("Search packages...");
    m_searchInput->setObjectName("searchInput");
    auto *btnSearch = new QPushButton("  Search");
    auto *btnClear  = new QPushButton("  Clear");
    btnSearch->setObjectName("btnPrimary");
    btnClear->setObjectName("btnSecondary");
    btnSearch->setFixedHeight(36);
    btnClear->setFixedHeight(36);
    searchRow->addWidget(m_searchInput);
    searchRow->addWidget(btnSearch);
    searchRow->addWidget(btnClear);
    topLayout->addLayout(searchRow);

    // Separator
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("separator");
    topLayout->addWidget(sep);

    // Filters
    auto *filtersLabel = new QLabel("  Filters");
    filtersLabel->setObjectName("sectionLabel");
    topLayout->addWidget(filtersLabel);

    auto *filterRow = new QHBoxLayout;
    filterRow->setSpacing(12);
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
    m_includeAur->setChecked(true);
    m_installedOnly = new QRadioButton("Installed Only");
    filterRow->addWidget(repoLabel);
    filterRow->addWidget(m_repoFilter);
    filterRow->addSpacing(12);
    filterRow->addWidget(statusLabel);
    filterRow->addWidget(m_statusFilter);
    filterRow->addSpacing(12);
    filterRow->addWidget(m_includeAur);
    filterRow->addWidget(m_installedOnly);
    filterRow->addStretch();
    topLayout->addLayout(filterRow);

    // Action buttons
    auto *actionRow  = new QHBoxLayout;
    auto *btnInstall = new QPushButton("  Install Selected");
    auto *btnRemove  = new QPushButton("  Remove Selected");
    auto *btnSelAll  = new QPushButton("  Select All");
    auto *btnClrSel  = new QPushButton("  Clear Selection");
    btnInstall->setObjectName("btnInstall");
    btnRemove->setObjectName("btnRemove");
    btnSelAll->setObjectName("btnSecondary");
    btnClrSel->setObjectName("btnSecondary");
    for (auto *b : {btnInstall, btnRemove, btnSelAll, btnClrSel})
        b->setFixedHeight(34);
    actionRow->addWidget(btnInstall);
    actionRow->addWidget(btnRemove);
    actionRow->addStretch();
    actionRow->addWidget(btnSelAll);
    actionRow->addWidget(btnClrSel);
    topLayout->addLayout(actionRow);

    rootLayout->addWidget(topSection);

    // ── Splitter: package list (top) + terminal tabs (bottom) ─────────────────
    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("mainSplitter");
    splitter->setChildrenCollapsible(false);

    // Package list area
    auto *packageArea = new QFrame;
    packageArea->setObjectName("packageArea");
    packageArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *pkgLayout = new QVBoxLayout(packageArea);
    pkgLayout->setAlignment(Qt::AlignCenter);
    auto *emptyIcon = new QLabel("ℹ");
    emptyIcon->setObjectName("emptyIcon");
    emptyIcon->setAlignment(Qt::AlignCenter);
    m_emptyLabel = new QLabel("No packages found");
    m_emptyLabel->setObjectName("emptyLabel");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    auto *emptyHint = new QLabel("Try adjusting your search criteria or filters");
    emptyHint->setObjectName("emptyHint");
    emptyHint->setAlignment(Qt::AlignCenter);
    pkgLayout->addWidget(emptyIcon);
    pkgLayout->addWidget(m_emptyLabel);
    pkgLayout->addWidget(emptyHint);
    splitter->addWidget(packageArea);

    // Terminal tab widget
    m_bottomTabs = new QTabWidget;
    m_bottomTabs->setObjectName("terminalTabs");
    m_bottomTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_terminal = new TerminalWidget;
    m_terminal->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bottomTabs->addTab(m_terminal, "  Terminal");

    m_outputView = new QPlainTextEdit;
    m_outputView->setObjectName("terminalView");
    m_outputView->setReadOnly(true);
    m_outputView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outputView->setPlaceholderText("Operation output will appear here...");
    m_bottomTabs->addTab(m_outputView, "  Output");

    splitter->addWidget(m_bottomTabs);
    splitter->setSizes({450, 250});

    rootLayout->addWidget(splitter, 1);
}

void SearchPage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}