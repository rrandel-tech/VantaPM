#include "InstalledPage.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QScrollBar>
#include <QHeaderView>
#include <QCheckBox>

// ── Helpers ───────────────────────────────────────────────────────────────────

namespace {

// Creates a fully-populated table row.
// col0 = radio/select  col1 = pkg icon + name  col2 = version
// col3 = repo icon     col4 = description       col5/6 = action buttons
void addPackageRow(QTableWidget *table, int row,
                   const QString &name,
                   const QString &version,
                   const QString &repo,
                   const QString &description)
{
    table->insertRow(row);
    table->setRowHeight(row, 46);

    // ── Col 0 : radio-style select indicator ─────────────────────────────────
    auto *selectWidget = new QWidget;
    auto *selectLayout = new QHBoxLayout(selectWidget);
    selectLayout->setContentsMargins(8, 0, 0, 0);
    selectLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    auto *radio = new QCheckBox;
    radio->setObjectName("rowCheck");
    selectLayout->addWidget(radio);
    table->setCellWidget(row, 0, selectWidget);

    // ── Col 1 : pkg icon + name ───────────────────────────────────────────────
    auto *nameWidget = new QWidget;
    auto *nameLayout = new QHBoxLayout(nameWidget);
    nameLayout->setContentsMargins(6, 0, 6, 0);
    nameLayout->setSpacing(6);
    nameLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto *pkgIcon = new QLabel;
    pkgIcon->setObjectName("pkgIcon");
    pkgIcon->setPixmap(QPixmap(":/icons/light/installed.png")
                           .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *nameLabel = new QLabel(name);
    nameLabel->setObjectName("pkgName");
    nameLayout->addWidget(pkgIcon);
    nameLayout->addWidget(nameLabel);
    table->setCellWidget(row, 1, nameWidget);

    // ── Col 2 : version ───────────────────────────────────────────────────────
    auto *versionLabel = new QLabel(version);
    versionLabel->setObjectName("pkgVersion");
    versionLabel->setContentsMargins(6, 0, 6, 0);
    versionLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    table->setCellWidget(row, 2, versionLabel);

    // ── Col 3 : repo icon ─────────────────────────────────────────────────────
    auto *repoWidget = new QWidget;
    auto *repoLayout = new QHBoxLayout(repoWidget);
    repoLayout->setContentsMargins(6, 0, 6, 0);
    repoLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    auto *repoIcon = new QLabel;
    repoIcon->setObjectName("repoIcon");
    repoIcon->setPixmap(QPixmap(":/icons/light/repository.png")
                            .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    repoIcon->setToolTip(repo);
    repoLayout->addWidget(repoIcon);
    table->setCellWidget(row, 3, repoWidget);

    // ── Col 4 : description ───────────────────────────────────────────────────
    auto *descLabel = new QLabel(description);
    descLabel->setObjectName("pkgDesc");
    descLabel->setContentsMargins(6, 0, 6, 0);
    descLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    table->setCellWidget(row, 4, descLabel);

    // ── Col 5 : Details button ────────────────────────────────────────────────
    auto *btnDetails = new QPushButton("Details");
    btnDetails->setObjectName("btnSecondary");
    btnDetails->setFixedHeight(28);
    btnDetails->setCursor(Qt::PointingHandCursor);

    auto *detailsWrapper = new QWidget;
    auto *detailsLayout  = new QHBoxLayout(detailsWrapper);
    detailsLayout->setContentsMargins(4, 0, 4, 0);
    detailsLayout->addWidget(btnDetails);
    table->setCellWidget(row, 5, detailsWrapper);

    // ── Col 6 : Remove button ─────────────────────────────────────────────────
    auto *btnRemove = new QPushButton("Remove");
    btnRemove->setObjectName("btnRemove");
    btnRemove->setFixedHeight(28);
    btnRemove->setCursor(Qt::PointingHandCursor);

    auto *removeWrapper = new QWidget;
    auto *removeLayout  = new QHBoxLayout(removeWrapper);
    removeLayout->setContentsMargins(4, 0, 8, 0);
    removeLayout->addWidget(btnRemove);
    table->setCellWidget(row, 6, removeWrapper);
}

} // namespace

// ── InstalledPage ─────────────────────────────────────────────────────────────

InstalledPage::InstalledPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void InstalledPage::setupUi()
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

    // Title row
    auto *titleRow = new QHBoxLayout;
    auto *titleIcon = new QLabel;
    titleIcon->setPixmap(QPixmap(":/icons/light/installed.png")
                             .scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *pageTitle = new QLabel("Installed Packages");
    pageTitle->setObjectName("pageTitle");
    titleRow->addWidget(titleIcon);
    titleRow->addWidget(pageTitle);
    titleRow->addStretch();
    topLayout->addLayout(titleRow);

    // Search bar
    auto *searchRow = new QHBoxLayout;
    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText("Search installed packages...");
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

    // Action buttons
    auto *actionRow = new QHBoxLayout;

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

    // ── Package table ─────────────────────────────────────────────────────────
    m_table = new QTableWidget(0, 7);
    m_table->setObjectName("packageTable");
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setObjectName("pkgHeader");

    // Header labels — col 0 has the master checkbox
    auto *masterCheck = new QCheckBox;
    masterCheck->setObjectName("rowCheck");

    m_table->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(1, new QTableWidgetItem("Name"));
    m_table->setHorizontalHeaderItem(2, new QTableWidgetItem("Version"));
    m_table->setHorizontalHeaderItem(3, new QTableWidgetItem("Repository"));
    m_table->setHorizontalHeaderItem(4, new QTableWidgetItem("Description"));
    m_table->setHorizontalHeaderItem(5, new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(6, new QTableWidgetItem(""));

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);

    m_table->setColumnWidth(0, 48);
    m_table->setColumnWidth(3, 60);
    m_table->setColumnWidth(5, 100);
    m_table->setColumnWidth(6, 100);

    // Place master checkbox in header col 0
    m_table->horizontalHeader()->setDefaultSectionSize(46);
    m_table->setCornerButtonEnabled(false);

    populateDemoRows();

    splitter->addWidget(m_table);

    // ── Terminal output panel ─────────────────────────────────────────────────
    auto *outputPanel = new QWidget;
    outputPanel->setObjectName("outputPanel");
    auto *outputLayout = new QVBoxLayout(outputPanel);
    outputLayout->setContentsMargins(10, 8, 10, 8);
    outputLayout->setSpacing(6);

    auto *outputHeaderRow = new QHBoxLayout;
    m_outputIcon = new QLabel;
    m_outputIcon->setPixmap(QPixmap(":/icons/light/terminal.png")
                                .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_outputLabel = new QLabel("Terminal Output");
    m_outputLabel->setObjectName("sectionLabel");
    outputHeaderRow->addWidget(m_outputIcon);
    outputHeaderRow->addWidget(m_outputLabel);
    outputHeaderRow->addStretch();
    outputLayout->addLayout(outputHeaderRow);

    m_outputView = new QPlainTextEdit;
    m_outputView->setObjectName("terminalView");
    m_outputView->setReadOnly(true);
    m_outputView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outputView->setPlaceholderText("Terminal output will appear here...");
    m_outputView->setFrameShape(QFrame::NoFrame);
    outputLayout->addWidget(m_outputView);

    splitter->addWidget(outputPanel);
    splitter->setSizes({520, 160});

    rootLayout->addWidget(splitter, 1);
}

void InstalledPage::populateDemoRows()
{
    struct PkgInfo { const char *name; const char *ver; const char *repo; const char *desc; };

    static constexpr PkgInfo pkgs[] = {
        { "7zip",                "25.01-1.1",      "extra",   "" },
        { "a52dec",              "0.8.0-2.1",      "extra",   "" },
        { "aalib",               "1.4rc5-19.1",    "extra",   "" },
        { "abseil-cpp",          "20250814.1-1.1", "extra",   "" },
        { "accounts-qml-module", "0.7-6.1",        "extra",   "" },
        { "acl",                 "2.3.2-1",        "core",    "" },
        { "acpi",                "1.7-4",          "extra",   "" },
        { "acpid",               "2.0.34-2",       "extra",   "" },
    };

    for (int i = 0; const auto &p : pkgs)
        addPackageRow(m_table, i++, p.name, p.ver, p.repo, p.desc);
}

void InstalledPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";

    m_btnSearch->setIcon(QIcon(prefix + "search.png"));
    m_btnClear->setIcon(QIcon(prefix + "clear.png"));
    m_btnRemove->setIcon(QIcon(prefix + "remove.png"));
    m_btnClrSel->setIcon(QIcon(prefix + "clear.png"));
    m_outputIcon->setPixmap(QPixmap(prefix + "terminal.png")
                                .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Update all row pkg/repo icons
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (auto *w = m_table->cellWidget(row, 1))
            if (auto *icon = w->findChild<QLabel *>("pkgIcon"))
                icon->setPixmap(QPixmap(prefix + "installed.png")
                                    .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        if (auto *w = m_table->cellWidget(row, 3))
            if (auto *icon = w->findChild<QLabel *>("repoIcon"))
                icon->setPixmap(QPixmap(prefix + "repository.png")
                                    .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}