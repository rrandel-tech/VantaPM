#include "UpdatePage.hpp"
#include "UI/UpdateConfirmDialog.hpp"
#include "Backend/PacmanBackend.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QScrollBar>
#include <QCheckBox>
#include <QHeaderView>
#include <QStackedWidget>
#include <QCheckBox>

// ── Column indices ────────────────────────────────────────────────────────────
static constexpr int kColCheck   = 0;   // checkbox
static constexpr int kColName    = 1;
static constexpr int kColCurVer  = 2;
static constexpr int kColNewVer  = 3;
static constexpr int kColRepo    = 4;
static constexpr int kColCount   = 5;

UpdatePage::UpdatePage(QWidget *parent)
    : QWidget(parent)
    , m_backend(new PacmanBackend(this))
{
    setupUi();

    connect(m_backend, &PacmanBackend::upgradablePackages,
            this, &UpdatePage::onUpgradablePackages);
    connect(m_backend, &PacmanBackend::outputLine,
            this, &UpdatePage::onOutputLine);
    connect(m_backend, &PacmanBackend::finished,
            this, &UpdatePage::onFinished);
    connect(m_backend, &PacmanBackend::startError, this, [this](const QString &msg) {
        appendOutput(QStringLiteral("[error] ") + msg);
        setButtonsBusy(false);
    });
}

// ── UI setup ──────────────────────────────────────────────────────────────────

void UpdatePage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Action buttons ────────────────────────────────────────────────────────
    auto *btnBar = new QHBoxLayout;
    btnBar->setContentsMargins(0, 0, 0, 8);
    btnBar->setSpacing(6);

    auto makeBtn = [&](const QString &label, const QString &obj) {
        auto *b = new QPushButton(label);
        b->setObjectName(obj);
        b->setFixedHeight(32);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };

    m_btnCheck     = makeBtn("⟳  Check for Updates",  "btnSecondary");
    m_btnUpdate    = makeBtn("⬆  Update System",       "btnPrimary");
    m_btnAurCheck  = makeBtn("⟳  Check AUR Updates",  "btnSecondary");
    m_btnAurUpdate = makeBtn("⬆  Update AUR",          "btnSecondary");

    // AUR buttons are stubs — disabled until implemented
    m_btnAurCheck->setEnabled(false);
    m_btnAurUpdate->setEnabled(false);
    m_btnAurCheck->setToolTip("AUR support coming soon");
    m_btnAurUpdate->setToolTip("AUR support coming soon");

    btnBar->addWidget(m_btnCheck);
    btnBar->addWidget(m_btnUpdate);
    btnBar->addWidget(m_btnAurCheck);
    btnBar->addWidget(m_btnAurUpdate);
    btnBar->addStretch();

    root->addLayout(btnBar);

    // ── Status bar ────────────────────────────────────────────────────────────
    auto *statusRow = new QHBoxLayout;
    statusRow->setContentsMargins(2, 0, 0, 8);
    statusRow->setSpacing(6);

    m_statusIcon = new QLabel("ⓘ");
    m_statusIcon->setObjectName("updateStatusIcon");
    m_statusLabel = new QLabel(
        "No updates available. Click 'Check for Updates' or 'Check AUR Updates' to check for package upgrades.");
    m_statusLabel->setObjectName("updateStatusLabel");

    statusRow->addWidget(m_statusIcon);
    statusRow->addWidget(m_statusLabel);
    statusRow->addStretch();
    root->addLayout(statusRow);

    // ── Splitter: table area + output pane ────────────────────────────────────
    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("mainSplitter");
    splitter->setChildrenCollapsible(false);

    // ── Stacked widget: empty state vs table ──────────────────────────────────
    auto *stack = new QStackedWidget;
    stack->setObjectName("updateStack");

    // Empty state
    m_emptyWidget = new QWidget;
    {
        auto *el = new QVBoxLayout(m_emptyWidget);
        el->setAlignment(Qt::AlignCenter);
        el->setSpacing(8);

        auto *ico = new QLabel("ⓘ");
        ico->setObjectName("updateEmptyIcon");
        ico->setAlignment(Qt::AlignCenter);

        m_emptyLabel = new QLabel("No updates available");
        m_emptyLabel->setObjectName("emptyLabel");
        m_emptyLabel->setAlignment(Qt::AlignCenter);

        m_emptyHint = new QLabel(
            "Click 'Check for Updates' to check for available package upgrades.");
        m_emptyHint->setObjectName("emptyHint");
        m_emptyHint->setAlignment(Qt::AlignCenter);

        el->addWidget(ico);
        el->addWidget(m_emptyLabel);
        el->addWidget(m_emptyHint);
    }
    stack->addWidget(m_emptyWidget);   // index 0

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
    m_table->horizontalHeader()->setObjectName("pkgHeader");
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setCornerButtonEnabled(false);

    m_table->setHorizontalHeaderItem(kColCheck,  new QTableWidgetItem(""));
    m_table->setHorizontalHeaderItem(kColName,   new QTableWidgetItem("Package Name"));
    m_table->setHorizontalHeaderItem(kColCurVer, new QTableWidgetItem("Current Version"));
    m_table->setHorizontalHeaderItem(kColNewVer, new QTableWidgetItem("New Version"));
    m_table->setHorizontalHeaderItem(kColRepo,   new QTableWidgetItem("Repository"));

    m_table->horizontalHeader()->setSectionResizeMode(kColCheck,  QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(kColName,   QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColCurVer, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColNewVer, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColRepo,   QHeaderView::ResizeToContents);
    m_table->setColumnWidth(kColCheck, 40);

    stack->addWidget(m_table);         // index 1

    splitter->addWidget(stack);

    // ── Output pane ───────────────────────────────────────────────────────────
    auto *outputPanel = new QWidget;
    outputPanel->setObjectName("outputPanel");
    auto *outLayout = new QVBoxLayout(outputPanel);
    outLayout->setContentsMargins(0, 8, 0, 0);
    outLayout->setSpacing(4);

    auto *outHdr = new QHBoxLayout;
    m_outputLabel = new QLabel("Terminal Output");
    m_outputLabel->setObjectName("sectionLabel");
    outHdr->addWidget(m_outputLabel);
    outHdr->addStretch();
    outLayout->addLayout(outHdr);

    m_outputView = new QPlainTextEdit;
    m_outputView->setObjectName("terminalView");
    m_outputView->setReadOnly(true);
    m_outputView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outputView->setPlaceholderText("Terminal output will appear here during operations...");
    m_outputView->setFrameShape(QFrame::NoFrame);
    outLayout->addWidget(m_outputView);

    m_btnClearOut = new QPushButton("🗑  Clear Output");
    m_btnClearOut->setObjectName("btnSecondary");
    m_btnClearOut->setFixedHeight(30);
    m_btnClearOut->setMaximumWidth(140);
    outLayout->addWidget(m_btnClearOut);

    splitter->addWidget(outputPanel);
    splitter->setSizes({480, 200});

    root->addWidget(splitter, 1);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(m_btnCheck,    &QPushButton::clicked, this, &UpdatePage::onCheckUpdates);
    connect(m_btnUpdate,   &QPushButton::clicked, this, &UpdatePage::onUpdateSystem);
    connect(m_btnClearOut, &QPushButton::clicked, this, [this]() {
        m_outputView->clear();
    });

    // Show empty state initially
    stack->setCurrentIndex(0);

    // Store stack ref for later toggling
    m_table->setProperty("stack", QVariant::fromValue(static_cast<void*>(stack)));
}

// ── Slot implementations ──────────────────────────────────────────────────────

void UpdatePage::onCheckUpdates()
{
    if (m_backend->isBusy())
        return;

    m_currentOp = Op::Check;
    setButtonsBusy(true);
    m_outputView->clear();
    appendOutput("Starting update check...");

    m_backend->checkUpdates();
}

void UpdatePage::onUpdateSystem()
{
    if (m_upgradable.isEmpty()) {
        appendOutput("No updates available. Run 'Check for Updates' first.");
        return;
    }

    // Show confirmation dialog — sysUpgrade only starts if user confirms
    UpdateConfirmDialog dlg(m_upgradable, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_currentOp = Op::Upgrade;
    setButtonsBusy(true);
    appendOutput("Starting system upgrade...");

    m_backend->sysUpgrade();
}

void UpdatePage::onUpgradablePackages(const QList<Package> &packages)
{
    m_upgradable = packages;
    populateTable(packages);

    const int n = packages.size();
    emit statusMessage(n == 0
        ? QStringLiteral("  No updates available")
        : QStringLiteral("  Found %1 update(s) available").arg(n));
    if (n == 0) {
        m_statusIcon->setText("ⓘ");
        m_statusLabel->setText(
            "No updates available. Click 'Check for Updates' or 'Check AUR Updates' to check for package upgrades.");
    } else {
        m_statusIcon->setText("⟳");
        m_statusLabel->setText(
            QStringLiteral("%1 update%2 available (%3 system)")
                .arg(n)
                .arg(n == 1 ? "" : "s")
                .arg(n));
    }
}

void UpdatePage::onOutputLine(const QString &line)
{
    appendOutput(line);
}

void UpdatePage::onFinished(bool success, int exitCode)
{
    setButtonsBusy(false);

    if (!success && exitCode != 1) {
        appendOutput(QStringLiteral("[vantapm] operation failed (exit %1)").arg(exitCode));
    }

    if (m_currentOp == Op::Upgrade && success) {
        // After a successful upgrade, re-check so the table clears
        m_upgradable.clear();
        m_currentOp = Op::Check;
        setButtonsBusy(true);
        m_backend->checkUpdates();
        return;
    }

    m_currentOp = Op::None;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void UpdatePage::populateTable(const QList<Package> &packages)
{
    // Retrieve the QStackedWidget stored on the table
    auto *stack = static_cast<QStackedWidget *>(
        m_table->property("stack").value<void *>());

    if (packages.isEmpty()) {
        if (stack) stack->setCurrentIndex(0);
        return;
    }

    if (stack) stack->setCurrentIndex(1);

    m_table->setUpdatesEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(packages.size());

    for (int row = 0; row < packages.size(); ++row) {
        const Package &pkg = packages[row];
        m_table->setRowHeight(row, 40);

        // Col 0: checkbox
        auto *chkW = new QWidget;
        auto *chkL = new QHBoxLayout(chkW);
        chkL->setContentsMargins(0, 0, 0, 0);
        chkL->setAlignment(Qt::AlignCenter);
        auto *chk = new QCheckBox;
        chk->setObjectName("rowCheck");
        chkL->addWidget(chk);
        m_table->setCellWidget(row, kColCheck, chkW);

        auto makeItem = [](const QString &text, const QColor &fg = {}) {
            auto *item = new QTableWidgetItem(text);
            item->setFlags(Qt::ItemIsEnabled);
            if (fg.isValid()) item->setForeground(fg);
            return item;
        };

        m_table->setItem(row, kColName,   makeItem(pkg.name));
        m_table->setItem(row, kColCurVer, makeItem(pkg.version));
        m_table->setItem(row, kColNewVer, makeItem(pkg.newVersion, QColor("#4ec994")));

        // Repo: shown in a badge-style label
        auto *repoBadge = new QLabel(pkg.repo.isEmpty() ? "system" : pkg.repo);
        repoBadge->setObjectName("repoBadge");
        repoBadge->setAlignment(Qt::AlignCenter);
        repoBadge->setContentsMargins(8, 2, 8, 2);
        auto *badgeW = new QWidget;
        auto *badgeL = new QHBoxLayout(badgeW);
        badgeL->setContentsMargins(4, 0, 4, 0);
        badgeL->addWidget(repoBadge);
        m_table->setCellWidget(row, kColRepo, badgeW);
    }

    m_table->setUpdatesEnabled(true);
}

void UpdatePage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}

void UpdatePage::setButtonsBusy(bool busy)
{
    m_btnCheck->setEnabled(!busy);
    m_btnUpdate->setEnabled(!busy);
}

void UpdatePage::updateIcons(bool /*isDark*/)
{
    // Icon-based buttons not used here; nothing to swap
}