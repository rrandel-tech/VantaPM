#include "SearchPage.hpp"
#include "Backend/PacmanBackend.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QScrollBar>
#include <QHeaderView>
#include <QStackedWidget>

// ── Column indices ────────────────────────────────────────────────────────────
static constexpr int kColCheck  = 0;
static constexpr int kColIcon   = 1;
static constexpr int kColName   = 2;
static constexpr int kColVer    = 3;
static constexpr int kColRepo   = 4;
static constexpr int kColDesc   = 5;
static constexpr int kColStatus = 6;
static constexpr int kColDetail = 7;
static constexpr int kColAction = 8;
static constexpr int kColCount  = 9;

SearchPage::SearchPage(QWidget *parent)
    : QWidget(parent)
    , m_backend(new PacmanBackend(this))
{
    setupUi();

    connect(m_backend, &PacmanBackend::searchResults, this, &SearchPage::onSearchResults);
    connect(m_backend, &PacmanBackend::outputLine,    this, &SearchPage::onOutputLine);
    connect(m_backend, &PacmanBackend::finished,      this, &SearchPage::onFinished);
    connect(m_backend, &PacmanBackend::startError,    this, [this](const QString &msg) {
        appendOutput(QStringLiteral("[error] ") + msg);
    });
}

// ── UI setup ──────────────────────────────────────────────────────────────────

void SearchPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(12);

    // ── Top section ───────────────────────────────────────────────────────────
    auto *topSection = new QWidget;
    topSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *topLayout = new QVBoxLayout(topSection);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    // Title
    auto *titleRow  = new QHBoxLayout;
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
    m_searchInput   = new QLineEdit;
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

    m_includeAur = new QCheckBox("Include AUR");
    m_includeAur->setChecked(false);
    m_includeAur->setEnabled(false);

    m_installedOnly = new QRadioButton("Installed Only");

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

    // ── Package area (stacked: empty state / table) ───────────────────────────
    auto *stack = new QStackedWidget;

    // Empty state
    m_emptyState = new QFrame;
    m_emptyState->setObjectName("packageArea");
    m_emptyState->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *pkgLayout = new QVBoxLayout(m_emptyState);
    pkgLayout->setAlignment(Qt::AlignCenter);
    pkgLayout->setSpacing(6);

    m_emptyIcon = new QLabel;
    m_emptyIcon->setPixmap(QPixmap(":/icons/light/empty.png")
                               .scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

    // Package table
    m_table = new QTableWidget(0, kColCount);
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

    m_table->setHorizontalHeaderItem(kColCheck,  new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColIcon,   new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColName,   new QTableWidgetItem("Name"));
    m_table->setHorizontalHeaderItem(kColVer,    new QTableWidgetItem("Version"));
    m_table->setHorizontalHeaderItem(kColRepo,   new QTableWidgetItem("Repository"));
    m_table->setHorizontalHeaderItem(kColDesc,   new QTableWidgetItem("Description"));
    m_table->setHorizontalHeaderItem(kColStatus, new QTableWidgetItem("Status"));
    m_table->setHorizontalHeaderItem(kColDetail, new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColAction, new QTableWidgetItem(""));

    m_table->horizontalHeader()->setSectionResizeMode(kColCheck,  QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColIcon,   QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColName,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColVer,    QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColRepo,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColDesc,   QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColStatus, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColDetail, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColAction, QHeaderView::Fixed);

    m_table->setColumnWidth(kColCheck,  40);
    m_table->setColumnWidth(kColIcon,   32);
    m_table->setColumnWidth(kColStatus, 110);
    m_table->setColumnWidth(kColDetail, 90);
    m_table->setColumnWidth(kColAction, 90);

    stack->addWidget(m_emptyState);   // index 0
    stack->addWidget(m_table);        // index 1
    stack->setCurrentIndex(0);

    splitter->addWidget(stack);

    // ── Output pane ───────────────────────────────────────────────────────────
    auto *outputPanel = new QWidget;
    outputPanel->setObjectName("outputPanel");
    auto *outputLayout = new QVBoxLayout(outputPanel);
    outputLayout->setContentsMargins(10, 8, 10, 8);
    outputLayout->setSpacing(4);

    auto *outputHeaderRow = new QHBoxLayout;
    auto *outputIcon      = new QLabel;
    outputIcon->setPixmap(QPixmap(":/icons/light/output.png")
                              .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *outputLabel = new QLabel("Terminal Output");
    outputLabel->setObjectName("sectionLabel");
    outputHeaderRow->addWidget(outputIcon);
    outputHeaderRow->addWidget(outputLabel);
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
    splitter->setSizes({450, 200});

    rootLayout->addWidget(splitter, 1);

    // ── Signal / slot wiring ──────────────────────────────────────────────────
    connect(m_btnSearch,     &QPushButton::clicked,            this, &SearchPage::onSearch);
    connect(m_btnClear,      &QPushButton::clicked,            this, &SearchPage::onClear);
    connect(m_btnInstall,    &QPushButton::clicked,            this, &SearchPage::onInstallSelected);
    connect(m_btnRemove,     &QPushButton::clicked,            this, &SearchPage::onRemoveSelected);
    connect(m_btnSelAll,     &QPushButton::clicked,            this, &SearchPage::onSelectAll);
    connect(m_btnClrSel,     &QPushButton::clicked,            this, &SearchPage::onClearSelection);

    connect(m_installedOnly, &QRadioButton::toggled,           this, &SearchPage::onInstalledOnlyToggled);
    connect(m_repoFilter,    &QComboBox::currentIndexChanged,  this, &SearchPage::onRepoFilterChanged);
    connect(m_statusFilter,  &QComboBox::currentIndexChanged,  this, &SearchPage::onStatusFilterChanged);

    connect(m_searchInput,   &QLineEdit::returnPressed,        this, &SearchPage::onSearch);
}

// ── Table population ──────────────────────────────────────────────────────────

void SearchPage::populateTable(const QList<Package> &packages)
{
    auto *stack = qobject_cast<QStackedWidget *>(m_table->parentWidget());

    m_table->setRowCount(0);

    if (packages.isEmpty()) {
        if (stack) stack->setCurrentIndex(0);
        return;
    }

    if (stack) stack->setCurrentIndex(1);

    m_table->setUpdatesEnabled(false);

    for (const Package &pkg : packages) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setRowHeight(row, 42);

        // ── Col 0 : checkbox ──────────────────────────────────────────────────
        auto *chkWidget = new QWidget;
        auto *chkLayout = new QHBoxLayout(chkWidget);
        chkLayout->setContentsMargins(8, 0, 0, 0);
        chkLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        auto *chk = new QCheckBox;
        chk->setObjectName("rowCheck");
        chkLayout->addWidget(chk);
        m_table->setCellWidget(row, kColCheck, chkWidget);

        // ── Col 1 : package type icon ─────────────────────────────────────────
        auto *iconWidget = new QWidget;
        auto *iconLayout = new QHBoxLayout(iconWidget);
        iconLayout->setContentsMargins(4, 0, 0, 0);
        iconLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        auto *pkgIcon = new QLabel;
        pkgIcon->setObjectName("pkgIcon");
        pkgIcon->setPixmap(QPixmap(":/icons/light/installed.png")
                               .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLayout->addWidget(pkgIcon);
        m_table->setCellWidget(row, kColIcon, iconWidget);

        // ── Col 2 : name ──────────────────────────────────────────────────────
        auto *nameLabel = new QLabel(pkg.name);
        nameLabel->setObjectName("pkgName");
        nameLabel->setContentsMargins(4, 0, 6, 0);
        nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setCellWidget(row, kColName, nameLabel);

        // ── Col 3 : version ───────────────────────────────────────────────────
        auto *verLabel = new QLabel(pkg.version);
        verLabel->setObjectName("pkgVersion");
        verLabel->setContentsMargins(4, 0, 6, 0);
        verLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setCellWidget(row, kColVer, verLabel);

        // ── Col 4 : repo (icon + name) ────────────────────────────────────────
        auto *repoWidget = new QWidget;
        auto *repoLayout = new QHBoxLayout(repoWidget);
        repoLayout->setContentsMargins(4, 0, 6, 0);
        repoLayout->setSpacing(4);
        repoLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        auto *repoIcon = new QLabel;
        repoIcon->setObjectName("repoIcon");
        repoIcon->setPixmap(QPixmap(":/icons/light/repository.png")
                                .scaled(13, 13, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        auto *repoName = new QLabel(pkg.repo);
        repoName->setObjectName("pkgVersion");
        repoLayout->addWidget(repoIcon);
        repoLayout->addWidget(repoName);
        m_table->setCellWidget(row, kColRepo, repoWidget);

        // ── Col 5 : description ───────────────────────────────────────────────
        auto *descLabel = new QLabel(pkg.description);
        descLabel->setObjectName("pkgDesc");
        descLabel->setContentsMargins(4, 0, 6, 0);
        descLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setCellWidget(row, kColDesc, descLabel);

        // ── Col 6 : status badge ──────────────────────────────────────────────
        auto *statusWidget = new QWidget;
        auto *statusLayout = new QHBoxLayout(statusWidget);
        statusLayout->setContentsMargins(4, 0, 4, 0);
        statusLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        auto *statusLabel = new QLabel(pkg.installed ? "Installed" : "Not Installed");
        statusLabel->setObjectName(pkg.installed ? "statusInstalled" : "statusNotInstalled");
        statusLayout->addWidget(statusLabel);
        m_table->setCellWidget(row, kColStatus, statusWidget);

        // ── Col 7 : Details button ────────────────────────────────────────────
        auto *btnDetails = new QPushButton("Details");
        btnDetails->setObjectName("btnSecondary");
        btnDetails->setFixedHeight(26);
        btnDetails->setCursor(Qt::PointingHandCursor);
        const QString pkgName = pkg.name;
        const bool    isLocal = pkg.installed;
        connect(btnDetails, &QPushButton::clicked, this, [this, pkgName, isLocal]() {
            appendOutput(QStringLiteral("--- info: ") + pkgName + " ---");
            if (isLocal)
                m_backend->infoLocal(pkgName);
            else
                m_backend->infoSync(pkgName);
        });
        auto *detailWrapper = new QWidget;
        auto *detailLayout  = new QHBoxLayout(detailWrapper);
        detailLayout->setContentsMargins(4, 0, 4, 0);
        detailLayout->addWidget(btnDetails);
        m_table->setCellWidget(row, kColDetail, detailWrapper);

        // ── Col 8 : Install / Remove button ──────────────────────────────────
        auto *actionBtn = new QPushButton(pkg.installed ? "Remove" : "Install");
        actionBtn->setObjectName(pkg.installed ? "btnRemove" : "btnInstall");
        actionBtn->setFixedHeight(26);
        actionBtn->setCursor(Qt::PointingHandCursor);
        const bool installed = pkg.installed;
        connect(actionBtn, &QPushButton::clicked, this, [this, pkgName, installed]() {
            if (installed)
                m_backend->remove({pkgName});
            else
                m_backend->install({pkgName});
        });
        auto *actionWrapper = new QWidget;
        auto *actionLayout  = new QHBoxLayout(actionWrapper);
        actionLayout->setContentsMargins(4, 0, 8, 0);
        actionLayout->addWidget(actionBtn);
        m_table->setCellWidget(row, kColAction, actionWrapper);
    }

    m_table->setUpdatesEnabled(true);
}

// ── Filter logic ──────────────────────────────────────────────────────────────

void SearchPage::applyFilters()
{
    if (m_allResults.isEmpty())
        return;

    // Index 0 = "All Repositories" → empty string means no repo filter applied.
    const QString repoFilter    = m_repoFilter->currentIndex() == 0
                                      ? QString()
                                      : m_repoFilter->currentText().toLower();
    const int     statusIndex   = m_statusFilter->currentIndex();
    const bool    installedOnly = m_installedOnly->isChecked();

    QList<Package> filtered;
    filtered.reserve(m_allResults.size());

    for (const Package &pkg : m_allResults) {
        // ── Repo filter ───────────────────────────────────────────────────────
        // Only keep the package when its repo matches the selected repo.
        // An empty repoFilter (index 0) bypasses this check entirely.
        if (!repoFilter.isEmpty() && pkg.repo.toLower() != repoFilter)
            continue;

        // ── Installed Only radio ──────────────────────────────────────────────
        if (installedOnly && !pkg.installed)
            continue;

        // ── Status dropdown ───────────────────────────────────────────────────
        if (statusIndex == 1 && !pkg.installed)   continue;   // Installed
        if (statusIndex == 2 &&  pkg.installed)   continue;   // Not Installed
        if (statusIndex == 3 && !pkg.upgradable)  continue;   // Upgradable

        filtered.append(pkg);
    }

    populateTable(filtered);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void SearchPage::onSearch()
{
    const QString term = m_searchInput->text().trimmed();
    if (term.isEmpty())
        return;

    if (m_backend->isBusy()) {
        appendOutput(QStringLiteral("[vantapm] already running an operation"));
        return;
    }

    m_outputView->clear();
    appendOutput(QStringLiteral("Searching for: ") + term);
    m_backend->search(term);
}

void SearchPage::onClear()
{
    m_searchInput->clear();
    m_allResults.clear();
    m_table->setRowCount(0);

    if (auto *stack = qobject_cast<QStackedWidget *>(m_table->parentWidget()))
        stack->setCurrentIndex(0);

    m_outputView->clear();
    m_repoFilter->setCurrentIndex(0);
    m_statusFilter->setCurrentIndex(0);
    m_installedOnly->setChecked(false);
}

void SearchPage::onInstallSelected()
{
    QStringList targets;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk && chk->isChecked()) {
            if (auto *lbl = qobject_cast<QLabel *>(m_table->cellWidget(row, kColName)))
                targets << lbl->text();
        }
    }
    if (targets.isEmpty())
        return;

    appendOutput(QStringLiteral("Installing: ") + targets.join(", "));
    m_backend->install(targets);
}

void SearchPage::onRemoveSelected()
{
    QStringList targets;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk && chk->isChecked()) {
            if (auto *lbl = qobject_cast<QLabel *>(m_table->cellWidget(row, kColName)))
                targets << lbl->text();
        }
    }
    if (targets.isEmpty())
        return;

    appendOutput(QStringLiteral("Removing: ") + targets.join(", "));
    m_backend->remove(targets);
}

void SearchPage::onSelectAll()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk) chk->setChecked(true);
    }
}

void SearchPage::onClearSelection()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk) chk->setChecked(false);
    }
}

void SearchPage::onInstalledOnlyToggled(bool checked)
{
    if (checked)
        m_statusFilter->setCurrentIndex(0);
    applyFilters();
}

void SearchPage::onRepoFilterChanged(int /*index*/)
{
    applyFilters();
}

void SearchPage::onStatusFilterChanged(int index)
{
    if (index != 0)
        m_installedOnly->setChecked(false);
    applyFilters();
}

void SearchPage::onSearchResults(const QList<Package> &packages)
{
    m_allResults = packages;
    applyFilters();
    emit statusMessage(QStringLiteral("  Found %1 package(s)").arg(packages.size()));
    appendOutput(QStringLiteral("Found %1 package(s)").arg(packages.size()));
}

void SearchPage::onOutputLine(const QString &line)
{
    appendOutput(line);
}

void SearchPage::onFinished(bool success, int exitCode)
{
    if (!success)
        appendOutput(QStringLiteral("[vantapm] operation exited with code %1").arg(exitCode));
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void SearchPage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}

void SearchPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";

    m_btnSearch->setIcon(QIcon(prefix + "search.png"));
    m_btnClear->setIcon(QIcon(prefix + "clear.png"));
    m_btnInstall->setIcon(QIcon(prefix + "install.png"));
    m_btnRemove->setIcon(QIcon(prefix + "remove.png"));
    m_btnClrSel->setIcon(QIcon(prefix + "clear.png"));
    m_emptyIcon->setPixmap(QPixmap(prefix + "empty.png")
                               .scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (auto *w = m_table->cellWidget(row, kColIcon))
            if (auto *icon = w->findChild<QLabel *>("pkgIcon"))
                icon->setPixmap(QPixmap(prefix + "installed.png")
                                    .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        if (auto *w = m_table->cellWidget(row, kColRepo))
            if (auto *icon = w->findChild<QLabel *>("repoIcon"))
                icon->setPixmap(QPixmap(prefix + "repository.png")
                                    .scaled(13, 13, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}