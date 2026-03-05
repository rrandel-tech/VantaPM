#include "FlatpakPage.hpp"

#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QScrollBar>
#include <QCheckBox>
#include <QMessageBox>
#include <QDir>

static constexpr const char *kFlatpak = "/usr/bin/flatpak";

// ── Search table columns ──────────────────────────────────────────────────────
static constexpr int kSColCheck  = 0;
static constexpr int kSColName   = 1;
static constexpr int kSColAppId  = 2;
static constexpr int kSColVer    = 3;
static constexpr int kSColOrigin = 4;
static constexpr int kSColCount  = 5;

// ── Installed table columns ───────────────────────────────────────────────────
static constexpr int kIColName   = 0;
static constexpr int kIColAppId  = 1;
static constexpr int kIColVer    = 2;
static constexpr int kIColOrigin = 3;
static constexpr int kIColCount  = 4;

// ─────────────────────────────────────────────────────────────────────────────

FlatpakPage::FlatpakPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyRead,
            this, &FlatpakPage::onReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FlatpakPage::onProcessFinished);

    setupUi();
    refreshInstalledList();
}

// ── UI Construction ───────────────────────────────────────────────────────────

void FlatpakPage::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setObjectName("mainSplitter");
    splitter->setChildrenCollapsible(false);

    // ══════════════════════════════════════════════════════════════════════════
    // LEFT PANEL
    // ══════════════════════════════════════════════════════════════════════════
    auto *leftWidget = new QWidget;
    leftWidget->setMinimumWidth(400);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(6);

    // ── Search bar ────────────────────────────────────────────────────────────
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(6);

        m_searchInput = new QLineEdit;
        m_searchInput->setObjectName("searchInput");
        m_searchInput->setFixedHeight(36);
        m_searchInput->setPlaceholderText("Search for Flatpak packages...");

        m_btnSearch = new QPushButton("Search");
        m_btnSearch->setObjectName("btnPrimary");
        m_btnSearch->setFixedHeight(36);
        m_btnSearch->setMinimumWidth(80);

        row->addWidget(m_searchInput, 1);
        row->addWidget(m_btnSearch);
        leftLayout->addLayout(row);
    }

    // ── Search results: stacked (empty-state | table) ─────────────────────────
    m_searchStack = new QStackedWidget;

    // Index 0 — empty state (shown on first load and after clear)
    auto *emptyStateWidget = new QWidget;
    emptyStateWidget->setObjectName("packageArea");
    {
        auto *el = new QVBoxLayout(emptyStateWidget);
        el->setContentsMargins(0, 12, 0, 0);
        el->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        el->setSpacing(0);

        // Thin top border to visually frame the area like the screenshot
        auto *topLine = new QFrame;
        topLine->setFrameShape(QFrame::HLine);
        topLine->setObjectName("separator");
        el->addWidget(topLine);

        auto *inner = new QVBoxLayout;
        inner->setContentsMargins(12, 12, 12, 12);
        inner->setSpacing(6);
        inner->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        auto *row = new QHBoxLayout;
        row->setSpacing(8);
        auto *ico = new QLabel("ⓘ");
        ico->setObjectName("updateStatusIcon");
        m_searchEmptyLabel = new QLabel("No search results. Enter a search query and click Search.");
        m_searchEmptyLabel->setObjectName("pkgDesc");
        row->addWidget(ico);
        row->addWidget(m_searchEmptyLabel);
        row->addStretch();
        inner->addLayout(row);

        el->addLayout(inner);
        el->addStretch();
    }
    m_searchStack->addWidget(emptyStateWidget);  // index 0

    // Index 1 — results table
    m_searchTable = new QTableWidget(0, kSColCount);
    m_searchTable->setObjectName("packageTable");
    m_searchTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_searchTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_searchTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_searchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_searchTable->setShowGrid(false);
    m_searchTable->setAlternatingRowColors(false);
    m_searchTable->verticalHeader()->setVisible(false);
    m_searchTable->setFrameShape(QFrame::NoFrame);
    m_searchTable->setFocusPolicy(Qt::NoFocus);
    m_searchTable->horizontalHeader()->setObjectName("pkgHeader");
    m_searchTable->horizontalHeader()->setHighlightSections(false);
    m_searchTable->setCornerButtonEnabled(false);

    m_searchTable->setHorizontalHeaderItem(kSColCheck,  new QTableWidgetItem(""));
    m_searchTable->setHorizontalHeaderItem(kSColName,   new QTableWidgetItem("Name"));
    m_searchTable->setHorizontalHeaderItem(kSColAppId,  new QTableWidgetItem("Application ID"));
    m_searchTable->setHorizontalHeaderItem(kSColVer,    new QTableWidgetItem("Version"));
    m_searchTable->setHorizontalHeaderItem(kSColOrigin, new QTableWidgetItem("Origin"));

    m_searchTable->horizontalHeader()->setSectionResizeMode(kSColCheck,  QHeaderView::Fixed);
    m_searchTable->horizontalHeader()->setSectionResizeMode(kSColName,   QHeaderView::ResizeToContents);
    m_searchTable->horizontalHeader()->setSectionResizeMode(kSColAppId,  QHeaderView::Stretch);
    m_searchTable->horizontalHeader()->setSectionResizeMode(kSColVer,    QHeaderView::ResizeToContents);
    m_searchTable->horizontalHeader()->setSectionResizeMode(kSColOrigin, QHeaderView::ResizeToContents);
    m_searchTable->setColumnWidth(kSColCheck, 36);

    m_searchStack->addWidget(m_searchTable);     // index 1
    m_searchStack->setCurrentIndex(0);           // start on empty state

    leftLayout->addWidget(m_searchStack, 2);

    // ── Install Selected ──────────────────────────────────────────────────────
    m_btnInstallSelected = new QPushButton("⬇  Install Selected");
    m_btnInstallSelected->setObjectName("btnInstall");
    m_btnInstallSelected->setFixedHeight(34);
    m_btnInstallSelected->setCursor(Qt::PointingHandCursor);
    leftLayout->addWidget(m_btnInstallSelected);

    // ── Separator ─────────────────────────────────────────────────────────────
    {
        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setObjectName("separator");
        leftLayout->addWidget(sep);
    }

    // ── Installed Packages header ─────────────────────────────────────────────
    {
        auto *hdr = new QHBoxLayout;
        auto *ico = new QLabel("⬇");
        ico->setObjectName("sectionLabel");
        auto *lbl = new QLabel("Installed Packages");
        lbl->setObjectName("sectionLabel");
        hdr->addWidget(ico);
        hdr->addWidget(lbl);
        hdr->addStretch();
        leftLayout->addLayout(hdr);
    }

    // ── Installed filter ──────────────────────────────────────────────────────
    m_installedFilter = new QLineEdit;
    m_installedFilter->setObjectName("searchInput");
    m_installedFilter->setFixedHeight(32);
    m_installedFilter->setPlaceholderText("Filter installed packages...");
    leftLayout->addWidget(m_installedFilter);

    // ── Installed table ───────────────────────────────────────────────────────
    m_installedTable = new QTableWidget(0, kIColCount);
    m_installedTable->setObjectName("packageTable");
    m_installedTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_installedTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_installedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_installedTable->setShowGrid(false);
    m_installedTable->setAlternatingRowColors(false);
    m_installedTable->verticalHeader()->setVisible(false);
    m_installedTable->setFrameShape(QFrame::NoFrame);
    m_installedTable->setFocusPolicy(Qt::NoFocus);
    m_installedTable->horizontalHeader()->setObjectName("pkgHeader");
    m_installedTable->horizontalHeader()->setHighlightSections(false);
    m_installedTable->setCornerButtonEnabled(false);

    m_installedTable->setHorizontalHeaderItem(kIColName,   new QTableWidgetItem("Name"));
    m_installedTable->setHorizontalHeaderItem(kIColAppId,  new QTableWidgetItem("Application ID"));
    m_installedTable->setHorizontalHeaderItem(kIColVer,    new QTableWidgetItem("Version"));
    m_installedTable->setHorizontalHeaderItem(kIColOrigin, new QTableWidgetItem("Origin"));

    m_installedTable->horizontalHeader()->setSectionResizeMode(kIColName,   QHeaderView::ResizeToContents);
    m_installedTable->horizontalHeader()->setSectionResizeMode(kIColAppId,  QHeaderView::Stretch);
    m_installedTable->horizontalHeader()->setSectionResizeMode(kIColVer,    QHeaderView::ResizeToContents);
    m_installedTable->horizontalHeader()->setSectionResizeMode(kIColOrigin, QHeaderView::ResizeToContents);

    leftLayout->addWidget(m_installedTable, 3);

    splitter->addWidget(leftWidget);

    // ══════════════════════════════════════════════════════════════════════════
    // RIGHT PANEL — detail pane
    // ══════════════════════════════════════════════════════════════════════════
    auto *detailOuter = buildDetailPane();
    detailOuter->setMinimumWidth(260);
    detailOuter->setMaximumWidth(420);
    splitter->addWidget(detailOuter);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setSizes({900, 340});

    root->addWidget(splitter);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(m_btnSearch,          &QPushButton::clicked,      this, &FlatpakPage::onSearch);
    connect(m_searchInput,        &QLineEdit::returnPressed,  this, &FlatpakPage::onSearch);
    connect(m_btnInstallSelected, &QPushButton::clicked,      this, &FlatpakPage::onInstallSelected);
    connect(m_installedFilter,    &QLineEdit::textChanged,    this, &FlatpakPage::onFilterInstalled);
    connect(m_searchTable,        &QTableWidget::cellClicked, this, &FlatpakPage::onSearchRowClicked);
    connect(m_installedTable,     &QTableWidget::cellClicked, this, &FlatpakPage::onInstalledRowClicked);
}

QWidget *FlatpakPage::buildDetailPane()
{
    m_detailPane = new QWidget;
    m_detailPane->setObjectName("flatpakDetailPane");

    auto *outerLayout = new QVBoxLayout(m_detailPane);
    outerLayout->setContentsMargins(12, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_detailStack = new QStackedWidget;

    // ── Index 0: Placeholder ──────────────────────────────────────────────────
    auto *placeholder = new QWidget;
    {
        auto *pl = new QVBoxLayout(placeholder);
        pl->setAlignment(Qt::AlignCenter);
        pl->setSpacing(8);

        auto *ico = new QLabel("ⓘ");
        ico->setObjectName("updateEmptyIcon");
        ico->setAlignment(Qt::AlignCenter);

        auto *lbl = new QLabel("Select a package to view details");
        lbl->setObjectName("emptyLabel");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);

        pl->addWidget(ico);
        pl->addWidget(lbl);
    }
    m_detailStack->addWidget(placeholder);  // index 0

    // ── Index 1: Detail content ───────────────────────────────────────────────
    auto *detailScroll = new QScrollArea;
    detailScroll->setWidgetResizable(true);
    detailScroll->setFrameShape(QFrame::NoFrame);
    detailScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *detailContent = new QWidget;
    auto *detailLayout  = new QVBoxLayout(detailContent);
    detailLayout->setContentsMargins(0, 0, 8, 16);
    detailLayout->setSpacing(14);

    auto makeSectionHdr = [&](const QString &icon, const QString &title) -> QWidget * {
        auto *w = new QWidget;
        auto *l = new QHBoxLayout(w);
        l->setContentsMargins(0, 4, 0, 0);
        l->setSpacing(6);
        auto *ic = new QLabel(icon);
        ic->setObjectName("updateStatusIcon");
        auto *lb = new QLabel(title);
        lb->setObjectName("sectionLabel");
        lb->setStyleSheet("font-size:13px; font-weight:600;");
        l->addWidget(ic);
        l->addWidget(lb);
        l->addStretch();
        return w;
    };

    auto makeKV = [&](const QString &key, QLabel *&valueOut) -> QWidget * {
        auto *w = new QWidget;
        auto *l = new QHBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(8);
        auto *kLbl = new QLabel(key);
        kLbl->setObjectName("filterLabel");
        kLbl->setFixedWidth(88);
        valueOut = new QLabel("—");
        valueOut->setObjectName("pkgVersion");
        valueOut->setWordWrap(true);
        valueOut->setTextInteractionFlags(Qt::TextSelectableByMouse);
        l->addWidget(kLbl);
        l->addWidget(valueOut, 1);
        return w;
    };

    // Application Details
    detailLayout->addWidget(makeSectionHdr("ⓘ", "Application Details"));
    {
        auto *block = new QWidget;
        block->setObjectName("flatpakDetailBlock");
        auto *bl = new QVBoxLayout(block);
        bl->setContentsMargins(8, 8, 8, 8);
        bl->setSpacing(5);
        bl->addWidget(makeKV("Name:",         m_detailName));
        bl->addWidget(makeKV("Version:",      m_detailVersion));
        bl->addWidget(makeKV("Branch:",       m_detailBranch));
        bl->addWidget(makeKV("Origin:",       m_detailOrigin));
        bl->addWidget(makeKV("Installation:", m_detailInstall));
        bl->addWidget(makeKV("Runtime:",      m_detailRuntime));
        bl->addWidget(makeKV("Description:",  m_detailDescription));
        detailLayout->addWidget(block);
    }

    // Permissions
    detailLayout->addWidget(makeSectionHdr("ⓘ", "Permissions"));
    {
        auto *block = new QWidget;
        block->setObjectName("flatpakDetailBlock");
        auto *bl = new QVBoxLayout(block);
        bl->setContentsMargins(8, 8, 8, 8);
        bl->setSpacing(4);
        m_detailPermissions = new QLabel("No special permissions required");
        m_detailPermissions->setObjectName("pkgDesc");
        m_detailPermissions->setWordWrap(true);
        m_detailPermissions->setTextInteractionFlags(Qt::TextSelectableByMouse);
        bl->addWidget(m_detailPermissions);
        detailLayout->addWidget(block);
    }

    // Actions
    detailLayout->addWidget(makeSectionHdr("ⓘ", "Actions"));
    detailLayout->addWidget(buildActionsSection());

    detailLayout->addStretch();
    detailScroll->setWidget(detailContent);
    m_detailStack->addWidget(detailScroll);  // index 1

    m_detailStack->setCurrentIndex(0);
    outerLayout->addWidget(m_detailStack, 1);

    return m_detailPane;
}

QWidget *FlatpakPage::buildActionsSection()
{
    auto *block = new QWidget;
    auto *bl    = new QVBoxLayout(block);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(4);

    auto makeBtn = [&](const QString &icon, const QString &label,
                       const QString &objName) -> QPushButton * {
        auto *btn = new QPushButton(icon + "  " + label);
        btn->setObjectName(objName);
        btn->setFixedHeight(36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setEnabled(false); // disabled until an installed package is selected
        return btn;
    };

    m_btnManageData      = makeBtn("☰", "Manage User Data",  "flatpakActionBtn");
    m_btnUninstall       = makeBtn("−", "Uninstall",         "flatpakUninstallBtn");
    m_btnRemoveData      = makeBtn("⊗", "Remove Data",       "flatpakActionBtn");
    m_btnCreateSnapshot  = makeBtn("⊙", "Create Snapshot",   "flatpakActionBtn");
    m_btnRestoreSnapshot = makeBtn("⟳", "Restore Snapshot",  "flatpakActionBtn");

    bl->addWidget(m_btnManageData);
    bl->addWidget(m_btnUninstall);
    bl->addWidget(m_btnRemoveData);
    bl->addWidget(m_btnCreateSnapshot);
    bl->addWidget(m_btnRestoreSnapshot);

    connect(m_btnManageData,      &QPushButton::clicked, this, &FlatpakPage::onManageUserData);
    connect(m_btnUninstall,       &QPushButton::clicked, this, &FlatpakPage::onUninstall);
    connect(m_btnRemoveData,      &QPushButton::clicked, this, &FlatpakPage::onRemoveData);
    connect(m_btnCreateSnapshot,  &QPushButton::clicked, this, &FlatpakPage::onCreateSnapshot);
    connect(m_btnRestoreSnapshot, &QPushButton::clicked, this, &FlatpakPage::onRestoreSnapshot);

    return block;
}

// ── Process helpers ───────────────────────────────────────────────────────────

void FlatpakPage::runFlatpak(const QStringList &args)
{
    if (isBusy()) return;

    m_outputBuf.clear();
    m_process->start(kFlatpak, args);

    if (!m_process->waitForStarted(3000)) {
        emit statusMessage("  [error] flatpak not found or failed to start");
        m_currentOp = Op::None;
    }
}

bool FlatpakPage::isBusy() const
{
    return m_process->state() != QProcess::NotRunning;
}

// ── Table population ──────────────────────────────────────────────────────────

void FlatpakPage::populateSearchTable(const QList<FlatpakPackage> &pkgs)
{
    if (pkgs.isEmpty()) {
        // Stay on / return to empty state, update hint text
        m_searchEmptyLabel->setText("No packages found. Try a different search term.");
        m_searchStack->setCurrentIndex(0);
        emit statusMessage("  No packages found");
        return;
    }

    m_searchTable->setUpdatesEnabled(false);
    m_searchTable->clearContents();
    m_searchTable->setRowCount(pkgs.size());

    const QIcon pkgIcon(":/icons/light/flatpak.png");

    for (int row = 0; row < pkgs.size(); ++row) {
        const auto &pkg = pkgs[row];
        m_searchTable->setRowHeight(row, 36);

        // Checkbox
        auto *chkW = new QWidget;
        auto *chkL = new QHBoxLayout(chkW);
        chkL->setContentsMargins(0, 0, 0, 0);
        chkL->setAlignment(Qt::AlignCenter);
        auto *chk = new QCheckBox;
        chk->setObjectName("rowCheck");
        chkL->addWidget(chk);
        m_searchTable->setCellWidget(row, kSColCheck, chkW);

        auto makeItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text);
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            return it;
        };

        auto *nameItem = makeItem(pkg.name);
        nameItem->setIcon(pkgIcon);
        m_searchTable->setItem(row, kSColName,   nameItem);
        m_searchTable->setItem(row, kSColAppId,  makeItem(pkg.appId));
        m_searchTable->setItem(row, kSColVer,    makeItem(pkg.version));
        m_searchTable->setItem(row, kSColOrigin, makeItem(pkg.origin.isEmpty() ? "flathub" : pkg.origin));
    }

    m_searchTable->setUpdatesEnabled(true);
    m_searchStack->setCurrentIndex(1);  // show table
    emit statusMessage(QStringLiteral("  Found %1 package(s)").arg(pkgs.size()));
}

void FlatpakPage::populateInstalledTable(const QList<FlatpakPackage> &pkgs)
{
    const QString filter = m_installedFilter->text().trimmed().toLower();

    QList<FlatpakPackage> visible;
    visible.reserve(pkgs.size());
    for (const auto &pkg : pkgs)
        if (filter.isEmpty()
            || pkg.name.toLower().contains(filter)
            || pkg.appId.toLower().contains(filter))
            visible.append(pkg);

    m_installedTable->setUpdatesEnabled(false);
    m_installedTable->clearContents();
    m_installedTable->setRowCount(visible.size());

    const QIcon pkgIcon(":/icons/light/flatpak.png");

    for (int row = 0; row < visible.size(); ++row) {
        const auto &pkg = visible[row];
        m_installedTable->setRowHeight(row, 36);

        auto makeItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text);
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            return it;
        };

        auto *nameItem = makeItem(pkg.name);
        nameItem->setIcon(pkgIcon);
        m_installedTable->setItem(row, kIColName,   nameItem);
        m_installedTable->setItem(row, kIColAppId,  makeItem(pkg.appId));
        m_installedTable->setItem(row, kIColVer,    makeItem(pkg.version));
        m_installedTable->setItem(row, kIColOrigin, makeItem(pkg.origin.isEmpty() ? "flathub" : pkg.origin));
    }

    m_installedTable->setUpdatesEnabled(true);
}

// ── Detail pane ───────────────────────────────────────────────────────────────

void FlatpakPage::showDetailPlaceholder()
{
    m_detailStack->setCurrentIndex(0);
    m_selectedAppId.clear();

    for (auto *btn : {m_btnManageData, m_btnUninstall, m_btnRemoveData,
                      m_btnCreateSnapshot, m_btnRestoreSnapshot})
        btn->setEnabled(false);
}

void FlatpakPage::showDetailForPackage(const FlatpakPackage &pkg)
{
    m_selectedAppId = pkg.appId;

    m_detailName->setText(       pkg.name.isEmpty()         ? "—" : pkg.name);
    m_detailVersion->setText(    pkg.version.isEmpty()      ? "—" : pkg.version);
    m_detailBranch->setText(     pkg.branch.isEmpty()       ? "—" : pkg.branch);
    m_detailOrigin->setText(     pkg.origin.isEmpty()       ? "—" : pkg.origin);
    m_detailInstall->setText(    pkg.installation.isEmpty() ? "—" : pkg.installation);
    m_detailRuntime->setText(    pkg.runtime.isEmpty()      ? "—" : pkg.runtime);
    m_detailDescription->setText(pkg.description.isEmpty()  ? "—" : pkg.description);
    m_detailPermissions->setText(pkg.permissions.isEmpty()
                                     ? "No special permissions required"
                                     : pkg.permissions);

    const bool installed = pkg.installed;
    m_btnManageData->setEnabled(installed);
    m_btnUninstall->setEnabled(installed);
    m_btnRemoveData->setEnabled(installed);
    m_btnCreateSnapshot->setEnabled(installed);
    m_btnRestoreSnapshot->setEnabled(installed);

    m_detailStack->setCurrentIndex(1);
}

// ── Output parsing ────────────────────────────────────────────────────────────

void FlatpakPage::parseSearchOutput(const QString &raw)
{
    m_searchResults.clear();
    for (const QString &line : raw.split('\n')) {
        const QString t = line.trimmed();
        if (t.isEmpty()) continue;
        const QStringList cols = t.split('\t');
        if (cols.size() < 3) continue;

        FlatpakPackage pkg;
        pkg.name        = cols.value(0).trimmed();
        pkg.description = cols.value(1).trimmed();
        pkg.appId       = cols.value(2).trimmed();
        pkg.version     = cols.value(3).trimmed();
        pkg.branch      = cols.value(4).trimmed();
        pkg.origin      = cols.value(5).trimmed();
        pkg.installed   = false;
        m_searchResults.append(pkg);
    }
    populateSearchTable(m_searchResults);
}

void FlatpakPage::parseInstalledOutput(const QString &raw)
{
    m_installedPackages.clear();
    for (const QString &line : raw.split('\n')) {
        const QString t = line.trimmed();
        if (t.isEmpty()) continue;
        const QStringList cols = t.split('\t');
        if (cols.size() < 2) continue;

        FlatpakPackage pkg;
        pkg.name         = cols.value(0).trimmed();
        pkg.appId        = cols.value(1).trimmed();
        pkg.version      = cols.value(2).trimmed();
        pkg.branch       = cols.value(3).trimmed();
        pkg.installation = cols.value(4).trimmed();
        pkg.origin       = cols.value(5).trimmed();
        pkg.installed    = true;
        m_installedPackages.append(pkg);
    }
    populateInstalledTable(m_installedPackages);
}

static FlatpakPackage parseFlatpakInfo(const QString &raw)
{
    FlatpakPackage pkg;
    QStringList permissions;

    for (const QString &rawLine : raw.split('\n')) {
        const int colon = rawLine.indexOf(':');
        if (colon == -1) continue;
        const QString key   = rawLine.left(colon).trimmed().toLower();
        const QString value = rawLine.mid(colon + 1).trimmed();

        if      (key == "name")         pkg.name         = value;
        else if (key == "id")           pkg.appId        = value;
        else if (key == "version")      pkg.version      = value;
        else if (key == "branch")       pkg.branch       = value;
        else if (key == "origin")       pkg.origin       = value;
        else if (key == "installation") pkg.installation = value;
        else if (key == "runtime")      pkg.runtime      = value;
        else if (key == "subject")      pkg.description  = value;
        else if (key.contains("filesystem") ||
                 key.contains("socket")     ||
                 key.contains("device")     ||
                 key.contains("share")      ||
                 key.contains("talk"))
            permissions.append(rawLine.trimmed());
    }

    pkg.permissions = permissions.isEmpty()
                          ? "No special permissions required"
                          : permissions.join('\n');
    pkg.installed   = true;
    return pkg;
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void FlatpakPage::onSearch()
{
    const QString term = m_searchInput->text().trimmed();
    if (term.isEmpty() || isBusy()) return;

    m_currentOp = Op::Search;
    emit statusMessage("  Searching Flatpak...");
    runFlatpak({"search",
                "--columns=name,description,application,version,branch,remotes",
                term});
}

void FlatpakPage::onSearchRowClicked(int row, int /*col*/)
{
    if (row < 0 || row >= m_searchResults.size()) return;
    showDetailForPackage(m_searchResults[row]);
}

void FlatpakPage::onInstallSelected()
{
    if (isBusy()) return;

    QStringList targets;
    for (int r = 0; r < m_searchTable->rowCount(); ++r) {
        auto *w   = m_searchTable->cellWidget(r, kSColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk && chk->isChecked())
            if (auto *item = m_searchTable->item(r, kSColAppId))
                targets << item->text();
    }
    if (targets.isEmpty()) return;

    m_currentOp = Op::Install;
    emit statusMessage(QStringLiteral("  Installing %1 package(s)...").arg(targets.size()));

    QStringList args = {"install", "--noninteractive", "--assumeyes"};
    args += targets;
    runFlatpak(args);
}

void FlatpakPage::onFilterInstalled(const QString &/*text*/)
{
    populateInstalledTable(m_installedPackages);
}

void FlatpakPage::onInstalledRowClicked(int row, int /*col*/)
{
    const QString filter = m_installedFilter->text().trimmed().toLower();

    QList<FlatpakPackage> visible;
    for (const auto &pkg : m_installedPackages)
        if (filter.isEmpty()
            || pkg.name.toLower().contains(filter)
            || pkg.appId.toLower().contains(filter))
            visible.append(pkg);

    if (row < 0 || row >= visible.size()) return;

    const FlatpakPackage &pkg = visible[row];
    m_pendingAppId = pkg.appId;

    if (!pkg.runtime.isEmpty() || !pkg.description.isEmpty()) {
        showDetailForPackage(pkg);
        return;
    }

    m_currentOp = Op::FetchInfo;
    emit statusMessage(QStringLiteral("  Fetching info for %1...").arg(pkg.appId));
    runFlatpak({"info", pkg.appId});
}

void FlatpakPage::onManageUserData()
{
    if (m_selectedAppId.isEmpty()) return;
    QProcess::startDetached("xdg-open",
        {QDir::homePath() + "/.var/app/" + m_selectedAppId});
    emit statusMessage("  Opened user data directory");
}

void FlatpakPage::onUninstall()
{
    if (m_selectedAppId.isEmpty() || isBusy()) return;

    if (QMessageBox::question(this, "Uninstall",
            QStringLiteral("Uninstall %1?").arg(m_selectedAppId),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::Uninstall;
    emit statusMessage(QStringLiteral("  Uninstalling %1...").arg(m_selectedAppId));
    runFlatpak({"uninstall", "--noninteractive", "--assumeyes", m_selectedAppId});
}

void FlatpakPage::onRemoveData()
{
    if (m_selectedAppId.isEmpty() || isBusy()) return;

    if (QMessageBox::warning(this, "Remove Data",
            QStringLiteral("Permanently delete all user data for %1?\n"
                           "This cannot be undone.").arg(m_selectedAppId),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::RemoveData;
    emit statusMessage(QStringLiteral("  Removing data for %1...").arg(m_selectedAppId));
    runFlatpak({"uninstall", "--noninteractive", "--assumeyes",
                "--delete-data", m_selectedAppId});
}

void FlatpakPage::onCreateSnapshot()
{
    if (m_selectedAppId.isEmpty()) return;
    QMessageBox::information(this, "Create Snapshot",
        QStringLiteral("Snapshot support requires a Btrfs/ZFS subvolume setup.\n"
                       "Data directory: %1/.var/app/%2")
            .arg(QDir::homePath(), m_selectedAppId));
}

void FlatpakPage::onRestoreSnapshot()
{
    if (m_selectedAppId.isEmpty()) return;
    QMessageBox::information(this, "Restore Snapshot",
        "Snapshot restore requires manual Btrfs/ZFS subvolume rollback.\n"
        "Refer to your snapshot tool's documentation.");
}

// ── QProcess callbacks ────────────────────────────────────────────────────────

void FlatpakPage::onReadyRead()
{
    m_outputBuf += QString::fromLocal8Bit(m_process->readAll());
}

void FlatpakPage::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const bool    ok  = (status == QProcess::NormalExit) && (exitCode == 0);
    const QString raw = m_outputBuf;
    m_outputBuf.clear();

    const Op finished = m_currentOp;
    m_currentOp = Op::None;

    switch (finished) {

    case Op::Search:
        if (ok)
            parseSearchOutput(raw);
        else
            emit statusMessage("  Flatpak search failed");
        break;

    case Op::ListInstalled:
        if (ok)
            parseInstalledOutput(raw);
        else
            emit statusMessage("  Could not list installed Flatpak packages");
        break;

    case Op::FetchInfo:
        if (ok) {
            FlatpakPackage info = parseFlatpakInfo(raw);
            info.installed = true;
            for (auto &pkg : m_installedPackages) {
                if (pkg.appId == m_pendingAppId) {
                    pkg.runtime      = info.runtime;
                    pkg.description  = info.description;
                    pkg.permissions  = info.permissions;
                    if (!info.branch.isEmpty()) pkg.branch = info.branch;
                    info.name         = pkg.name;
                    info.version      = pkg.version;
                    info.origin       = pkg.origin;
                    info.installation = pkg.installation;
                    break;
                }
            }
            showDetailForPackage(info);
            emit statusMessage(QStringLiteral("  %1").arg(m_pendingAppId));
        }
        break;

    case Op::Install:
        emit statusMessage(ok ? "  Installation complete" : "  Installation failed");
        refreshInstalledList();
        break;

    case Op::Uninstall:
        if (ok) {
            emit statusMessage(QStringLiteral("  Uninstalled %1").arg(m_selectedAppId));
            showDetailPlaceholder();
        } else {
            emit statusMessage("  Uninstall failed");
        }
        refreshInstalledList();
        break;

    case Op::RemoveData:
        emit statusMessage(ok ? "  Data removed" : "  Remove data failed");
        refreshInstalledList();
        break;

    default:
        break;
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void FlatpakPage::refreshInstalledList()
{
    if (isBusy()) return;
    m_currentOp = Op::ListInstalled;
    runFlatpak({"list",
                "--columns=name,application,version,branch,installation,origin"});
}

void FlatpakPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";

    m_btnSearch->setIcon(QIcon(prefix + "search.png"));

    const QIcon pkgIcon(prefix + "flatpak.png");
    for (auto *table : {m_searchTable, m_installedTable}) {
        const int nameCol = (table == m_searchTable) ? kSColName : kIColName;
        for (int row = 0; row < table->rowCount(); ++row)
            if (auto *item = table->item(row, nameCol))
                item->setIcon(pkgIcon);
    }
}