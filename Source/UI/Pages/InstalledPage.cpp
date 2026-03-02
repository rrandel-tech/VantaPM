#include "InstalledPage.hpp"
#include "Backend/PacmanBackend.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QScrollBar>
#include <QHeaderView>

// ── Column indices ────────────────────────────────────────────────────────────
static constexpr int kColCheck  = 0;
static constexpr int kColName   = 1;
static constexpr int kColVer    = 2;
static constexpr int kColRepo   = 3;
static constexpr int kColDesc   = 4;
static constexpr int kColDetail = 5;
static constexpr int kColRemove = 6;
static constexpr int kColCount  = 7;

static constexpr int kCheckWidth  = 40;
static constexpr int kRepoWidth   = 110;
static constexpr int kDetailWidth = 75;
static constexpr int kRemoveWidth = 75;

// ── Construction ──────────────────────────────────────────────────────────────

InstalledPage::InstalledPage(QWidget *parent)
    : QWidget(parent)
    , m_fastBackend(new PacmanBackend(this))
    , m_fullBackend(new PacmanBackend(this))
{
    setupUi();

    // Fast backend: pacman -Q — name + version only, results in < 100 ms
    connect(m_fastBackend, &PacmanBackend::queryResults,
            this, &InstalledPage::onFastQueryResults);
    connect(m_fastBackend, &PacmanBackend::outputLine,
            this, &InstalledPage::onOutputLine);
    connect(m_fastBackend, &PacmanBackend::finished,
            this, &InstalledPage::onFinished);
    connect(m_fastBackend, &PacmanBackend::startError, this, [this](const QString &msg) {
        appendOutput(QStringLiteral("[error] ") + msg);
    });

    // Full backend: pacman -Qi — repo + description, fires after fast is done
    connect(m_fullBackend, &PacmanBackend::queryResults,
            this, &InstalledPage::onFullQueryResults);
    connect(m_fullBackend, &PacmanBackend::startError, this, [this](const QString &msg) {
        appendOutput(QStringLiteral("[error] ") + msg);
    });
}

void InstalledPage::loadPackages()
{
    if (m_fastBackend->isBusy() || m_fullBackend->isBusy())
        return;
    m_currentOp = Op::FastQuery;
    // Phase 1: get name+version list immediately, populate table with that
    m_fastBackend->queryInstalled();
}

// ── UI setup ──────────────────────────────────────────────────────────────────

void InstalledPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(12);

    auto *topSection = new QWidget;
    topSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *topLayout = new QVBoxLayout(topSection);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    // Title
    auto *titleRow  = new QHBoxLayout;
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
    m_searchInput   = new QLineEdit;
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
    m_table->setCornerButtonEnabled(false);
    m_table->setIconSize(QSize(14, 14));

    m_table->setHorizontalHeaderItem(kColCheck,  new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColName,   new QTableWidgetItem("Name"));
    m_table->setHorizontalHeaderItem(kColVer,    new QTableWidgetItem("Version"));
    m_table->setHorizontalHeaderItem(kColRepo,   new QTableWidgetItem("Repository"));
    m_table->setHorizontalHeaderItem(kColDesc,   new QTableWidgetItem("Description"));
    m_table->setHorizontalHeaderItem(kColDetail, new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColRemove, new QTableWidgetItem(""));

    m_table->horizontalHeader()->setSectionResizeMode(kColCheck,  QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColName,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColVer,    QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColRepo,   QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColDesc,   QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColDetail, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColRemove, QHeaderView::Fixed);

    m_table->setColumnWidth(kColCheck,  kCheckWidth);
    m_table->setColumnWidth(kColRepo,   kRepoWidth);
    m_table->setColumnWidth(kColDetail, kDetailWidth);
    m_table->setColumnWidth(kColRemove, kRemoveWidth);

    splitter->addWidget(m_table);

    // ── Output panel ─────────────────────────────────────────────────────────
    auto *outputPanel = new QWidget;
    outputPanel->setObjectName("outputPanel");
    auto *outputLayout = new QVBoxLayout(outputPanel);
    outputLayout->setContentsMargins(10, 8, 10, 8);
    outputLayout->setSpacing(4);

    auto *outputHeaderRow = new QHBoxLayout;
    m_outputIcon = new QLabel;
    m_outputIcon->setPixmap(QPixmap(":/icons/light/terminal.png")
                                .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *outputLabel = new QLabel("Terminal Output");
    outputLabel->setObjectName("sectionLabel");
    outputHeaderRow->addWidget(m_outputIcon);
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
    splitter->setSizes({520, 160});

    rootLayout->addWidget(splitter, 1);

    connect(m_btnSearch,   &QPushButton::clicked,     this, &InstalledPage::onSearch);
    connect(m_btnClear,    &QPushButton::clicked,     this, &InstalledPage::onClear);
    connect(m_btnRemove,   &QPushButton::clicked,     this, &InstalledPage::onRemoveSelected);
    connect(m_btnSelAll,   &QPushButton::clicked,     this, &InstalledPage::onSelectAll);
    connect(m_btnClrSel,   &QPushButton::clicked,     this, &InstalledPage::onClearSelection);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &InstalledPage::onSearch);
}

// ── Table population ──────────────────────────────────────────────────────────
// Full rebuild — only called for fast query results (name+version).
// Keeps widget count minimal: checkbox + 2 buttons per row = 3 widgets each.

void InstalledPage::populateTable(const QList<Package> &packages)
{
    const QIcon pkgIcon(m_isDark
                            ? QStringLiteral(":/icons/light/installed.png")
                            : QStringLiteral(":/icons/dark/installed.png"));

    m_table->setSortingEnabled(false);
    m_table->setUpdatesEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(packages.size());

    for (int row = 0; row < packages.size(); ++row) {
        const Package &pkg = packages[row];
        m_table->setRowHeight(row, 42);

        // ── Col 0 : checkbox ──────────────────────────────────────────────────
        auto *chkWidget = new QWidget;
        auto *chkLayout = new QHBoxLayout(chkWidget);
        chkLayout->setContentsMargins(0, 0, 0, 0);
        chkLayout->setAlignment(Qt::AlignCenter);
        auto *chk = new QCheckBox;
        chk->setObjectName("rowCheck");
        chkLayout->addWidget(chk);
        m_table->setCellWidget(row, kColCheck, chkWidget);

        // ── Col 1 : icon + name ───────────────────────────────────────────────
        auto *nameItem = new QTableWidgetItem(pkgIcon, pkg.name);
        nameItem->setFlags(Qt::ItemIsEnabled);
        m_table->setItem(row, kColName, nameItem);

        // ── Cols 2-4 : plain items (repo/desc initially empty) ────────────────
        auto makeItem = [](const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setFlags(Qt::ItemIsEnabled);
            return item;
        };
        m_table->setItem(row, kColVer,  makeItem(pkg.version));
        m_table->setItem(row, kColRepo, makeItem(QString()));
        m_table->setItem(row, kColDesc, makeItem(QString()));

        // ── Col 5 : Details button ────────────────────────────────────────────
        const QString pkgName = pkg.name;

        auto *btnDetails = new QPushButton("Details");
        btnDetails->setObjectName("btnSecondary");
        btnDetails->setFixedSize(kDetailWidth - 8, 26);
        btnDetails->setCursor(Qt::PointingHandCursor);
        connect(btnDetails, &QPushButton::clicked, this, [this, pkgName]() {
            m_currentOp = Op::Info;
            appendOutput(QStringLiteral("--- info: ") + pkgName + " ---");
            m_fastBackend->infoLocal(pkgName);
        });
        auto *dw = new QWidget;
        auto *dl = new QHBoxLayout(dw);
        dl->setContentsMargins(4, 0, 4, 0);
        dl->setSpacing(0);
        dl->addWidget(btnDetails);
        m_table->setCellWidget(row, kColDetail, dw);

        // ── Col 6 : Remove button ─────────────────────────────────────────────
        auto *btnRemove = new QPushButton("Remove");
        btnRemove->setObjectName("btnRemove");
        btnRemove->setFixedSize(kRemoveWidth - 8, 26);
        btnRemove->setCursor(Qt::PointingHandCursor);
        connect(btnRemove, &QPushButton::clicked, this, [this, pkgName]() {
            appendOutput(QStringLiteral("Removing: ") + pkgName);
            m_currentOp = Op::Remove;
            m_fastBackend->remove({pkgName});
        });
        auto *rw = new QWidget;
        auto *rl = new QHBoxLayout(rw);
        rl->setContentsMargins(4, 0, 4, 0);
        rl->setSpacing(0);
        rl->addWidget(btnRemove);
        m_table->setCellWidget(row, kColRemove, rw);
    }

    m_table->setUpdatesEnabled(true);
}

// ── Patch repo/description into existing rows ─────────────────────────────────
// Called when the full query finishes. Does NOT rebuild the table —
// just writes text into the existing QTableWidgetItems.

void InstalledPage::patchTableDetails(const QList<Package> &packages)
{
    // Build a name → Package map for O(1) lookup
    QHash<QString, const Package *> byName;
    byName.reserve(packages.size());
    for (const Package &pkg : packages)
        byName.insert(pkg.name, &pkg);

    m_table->setUpdatesEnabled(false);

    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *nameItem = m_table->item(row, kColName);
        if (!nameItem)
            continue;

        auto it = byName.constFind(nameItem->text());
        if (it == byName.constEnd())
            continue;

        const Package *pkg = it.value();

        if (auto *repoItem = m_table->item(row, kColRepo))
            repoItem->setText(pkg->repo.isEmpty()
                                  ? QStringLiteral("local")
                                  : pkg->repo);

        if (auto *descItem = m_table->item(row, kColDesc))
            descItem->setText(pkg->description);

        // Patch the in-memory list too so search works on descriptions
        // Find and update the corresponding entry in m_allPackages
        for (Package &ap : m_allPackages) {
            if (ap.name == pkg->name) {
                ap.repo        = pkg->repo;
                ap.description = pkg->description;
                break;
            }
        }
    }

    m_table->setUpdatesEnabled(true);
}

// ── Search ────────────────────────────────────────────────────────────────────

void InstalledPage::applySearch(const QString &term)
{
    if (term.isEmpty()) {
        populateTable(m_allPackages);
        emit statusMessage(QStringLiteral("  Found %1 installed package(s)").arg(m_allPackages.size()));
        return;
    }

    QList<Package> filtered;
    filtered.reserve(m_allPackages.size());
    for (const Package &pkg : m_allPackages) {
        if (pkg.name.contains(term, Qt::CaseInsensitive) ||
            pkg.description.contains(term, Qt::CaseInsensitive))
            filtered.append(pkg);
    }
    populateTable(filtered);
    emit statusMessage(QStringLiteral("  Found %1 matching package(s)").arg(filtered.size()));
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void InstalledPage::onSearch()
{
    applySearch(m_searchInput->text().trimmed());
}

void InstalledPage::onClear()
{
    m_searchInput->clear();
    populateTable(m_allPackages);
    m_outputView->clear();
}

void InstalledPage::onRemoveSelected()
{
    QStringList targets;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk && chk->isChecked()) {
            if (auto *item = m_table->item(row, kColName))
                targets << item->text();
        }
    }
    if (targets.isEmpty())
        return;

    appendOutput(QStringLiteral("Removing: ") + targets.join(", "));
    m_currentOp = Op::Remove;
    m_fastBackend->remove(targets);
}

void InstalledPage::onSelectAll()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk) chk->setChecked(true);
    }
}

void InstalledPage::onClearSelection()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto *w   = m_table->cellWidget(row, kColCheck);
        auto *chk = w ? w->findChild<QCheckBox *>() : nullptr;
        if (chk) chk->setChecked(false);
    }
}

void InstalledPage::onFastQueryResults(const QList<Package> &packages)
{
    m_allPackages = packages;
    populateTable(packages);
    emit statusMessage(QStringLiteral("  Found %1 installed package(s)").arg(packages.size()));
    // Phase 2: kick off the full query in the background now that the table
    // is already visible. The user sees the list immediately; repo/description
    // trickle in a second or two later without any visible freeze.
    if (!m_fullBackend->isBusy()) {
        m_currentOp = Op::FullQuery;
        m_fullBackend->queryInstalledFull();
    }
}

void InstalledPage::onFullQueryResults(const QList<Package> &packages)
{
    patchTableDetails(packages);
    m_currentOp = Op::None;
}

void InstalledPage::onOutputLine(const QString &line)
{
    // Only route to the output pane during user-initiated operations.
    // Suppress the noise from background pacman -Q / -Qi loads.
    if (m_currentOp == Op::Remove || m_currentOp == Op::Info)
        appendOutput(line);
}

void InstalledPage::onFinished(bool success, int exitCode)
{
    if (!success) {
        appendOutput(
            QStringLiteral("[vantapm] operation exited with code %1").arg(exitCode));
        m_currentOp = Op::None;
        return;
    }

    if (m_currentOp == Op::Remove) {
        // After a remove, restart the two-phase load
        m_currentOp = Op::FastQuery;
        m_fastBackend->queryInstalled();
    } else {
        m_currentOp = Op::None;
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void InstalledPage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}

void InstalledPage::updateIcons(bool isDark)
{
    m_isDark = isDark;
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";

    m_btnSearch->setIcon(QIcon(prefix + "search.png"));
    m_btnClear->setIcon(QIcon(prefix + "clear.png"));
    m_btnRemove->setIcon(QIcon(prefix + "remove.png"));
    m_btnClrSel->setIcon(QIcon(prefix + "clear.png"));
    m_outputIcon->setPixmap(QPixmap(prefix + "terminal.png")
                                .scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    const QIcon pkgIcon(prefix + "installed.png");
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (auto *item = m_table->item(row, kColName))
            item->setIcon(pkgIcon);
    }
}