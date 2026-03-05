#include "KernelPage.hpp"
#include "UI/KernelInstallDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QScrollBar>
#include <QMessageBox>
#include <QSplitter>
#include <QRegularExpression>

static constexpr const char *kPacman = "/usr/bin/pacman";
static constexpr const char *kPkexec = "/usr/bin/pkexec";

// ── Column indices — NO checkbox column, matches screenshot ──────────────────
static constexpr int kColName   = 0;
static constexpr int kColVer    = 1;
static constexpr int kColRepo   = 2;
static constexpr int kColDesc   = 3;
static constexpr int kColStatus = 4;
static constexpr int kColCount  = 5;

// Filter: only show actual kernel packages, not userspace tools
static bool isKernelPackage(const QString &name)
{
    static const QRegularExpression kernelRe(
        R"(^(linux(-(lts|zen|hardened|rt|rt-lts|mainline))?)$)"
    );

    return kernelRe.match(name).hasMatch();
}

// ─────────────────────────────────────────────────────────────────────────────

KernelPage::KernelPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyRead,
            this, &KernelPage::onReadyRead);
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &KernelPage::onProcessFinished);

    setupUi();
    loadKernels();
}

// ── UI Construction ───────────────────────────────────────────────────────────

void KernelPage::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("mainSplitter");
    splitter->setChildrenCollapsible(false);

    // ── Table area ────────────────────────────────────────────────────────────
    auto *tableWidget = new QWidget;
    auto *tableLayout = new QVBoxLayout(tableWidget);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(0);

    m_table = new QTableWidget(0, kColCount);
    m_table->setObjectName("packageTable");
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Multi-row selection so the user can select several kernels at once
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->setFocusPolicy(Qt::StrongFocus);
    m_table->horizontalHeader()->setObjectName("pkgHeader");
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setCornerButtonEnabled(false);

    m_table->setHorizontalHeaderItem(kColName,   new QTableWidgetItem("Name"));
    m_table->setHorizontalHeaderItem(kColVer,    new QTableWidgetItem("Version"));
    m_table->setHorizontalHeaderItem(kColRepo,   new QTableWidgetItem("Repository"));
    m_table->setHorizontalHeaderItem(kColDesc,   new QTableWidgetItem("Description"));
    m_table->setHorizontalHeaderItem(kColStatus, new QTableWidgetItem("Status"));

    m_table->horizontalHeader()->setSectionResizeMode(kColName,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColVer,    QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColRepo,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColDesc,   QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColStatus, QHeaderView::ResizeToContents);

    tableLayout->addWidget(m_table, 1);

    // ── Footer: Install Selected | Remove Selected ─────────────────────────
    auto *footer = new QHBoxLayout;
    footer->setSpacing(0);
    footer->setContentsMargins(0, 0, 0, 0);

    m_btnInstall = new QPushButton("⬇  Install Selected");
    m_btnInstall->setObjectName("kernelFooterBtnInstall");
    m_btnInstall->setFixedHeight(40);
    m_btnInstall->setCursor(Qt::PointingHandCursor);
    m_btnInstall->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_btnRemove = new QPushButton("⊗  Remove Selected");
    m_btnRemove->setObjectName("kernelFooterBtnRemove");
    m_btnRemove->setFixedHeight(40);
    m_btnRemove->setCursor(Qt::PointingHandCursor);
    m_btnRemove->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    footer->addWidget(m_btnInstall);
    footer->addWidget(m_btnRemove);
    tableLayout->addLayout(footer);

    splitter->addWidget(tableWidget);

    // ── Terminal Output pane ──────────────────────────────────────────────────
    auto *outputPanel = new QWidget;
    outputPanel->setObjectName("outputPanel");
    auto *outputLayout = new QVBoxLayout(outputPanel);
    outputLayout->setContentsMargins(0, 8, 0, 0);
    outputLayout->setSpacing(4);

    auto *outHdr = new QHBoxLayout;
    auto *outIco = new QLabel("⊙");
    outIco->setObjectName("updateStatusIcon");
    auto *outLbl = new QLabel("Terminal Output");
    outLbl->setObjectName("sectionLabel");
    outHdr->addWidget(outIco);
    outHdr->addWidget(outLbl);
    outHdr->addStretch();
    outputLayout->addLayout(outHdr);

    m_outputView = new QPlainTextEdit;
    m_outputView->setObjectName("terminalView");
    m_outputView->setReadOnly(true);
    m_outputView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outputView->setPlaceholderText("Kernel installation and removal progress will appear here...");
    m_outputView->setFrameShape(QFrame::NoFrame);
    outputLayout->addWidget(m_outputView);

    splitter->addWidget(outputPanel);
    splitter->setSizes({520, 160});

    root->addWidget(splitter, 1);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_btnInstall, &QPushButton::clicked, this, &KernelPage::onInstallSelected);
    connect(m_btnRemove,  &QPushButton::clicked, this, &KernelPage::onRemoveSelected);
}

// ── Data loading (two-phase) ──────────────────────────────────────────────────

void KernelPage::loadKernels()
{
    m_kernels.clear();
    m_installedNames.clear();
    m_currentOp = Op::QueryInstalled;
    m_outputBuf.clear();
    // Phase 1: fast installed list
    m_process->start(kPacman, {"-Q"});
}

void KernelPage::parseInstalledOutput(const QString &raw)
{
    for (const QString &line : raw.split('\n')) {
        const QString t = line.trimmed();
        if (t.isEmpty()) continue;
        const int sp = t.indexOf(' ');
        if (sp != -1)
            m_installedNames.append(t.left(sp));
    }
}

void KernelPage::parseSearchOutput(const QString &raw)
{
    m_kernels.clear();

    static const QRegularExpression headerRe(
        R"(^([^/]+)/(\S+)\s+(\S+)(.*))");

    KernelPackage current;
    bool pending = false;

    for (const QString &line : raw.split('\n')) {
        if (line.isEmpty()) continue;

        if (!line[0].isSpace()) {
            if (pending && isKernelPackage(current.name))
                m_kernels.append(current);

            current = {};
            pending = false;

            const auto m = headerRe.match(line);
            if (!m.hasMatch()) continue;

            current.repo      = m.captured(1).trimmed();
            current.name      = m.captured(2).trimmed();
            current.version   = m.captured(3).trimmed();
            current.installed = m_installedNames.contains(current.name);
            pending = true;
        } else if (pending) {
            current.description = line.trimmed();
        }
    }

    if (pending && isKernelPackage(current.name))
        m_kernels.append(current);

    populateTable(m_kernels);
    emit statusMessage(QStringLiteral("  Found %1 kernel(s)").arg(m_kernels.size()));
}

// ── Table population ──────────────────────────────────────────────────────────

void KernelPage::populateTable(const QList<KernelPackage> &kernels)
{
    m_table->setUpdatesEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(kernels.size());

    const QIcon kernelIcon(":/icons/light/kernel.png");

    for (int row = 0; row < kernels.size(); ++row) {
        const auto &k = kernels[row];
        m_table->setRowHeight(row, 42);

        auto makeItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text);
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            return it;
        };

        // Col 0 — name with kernel icon; installed rows get a checkmark prefix
        auto *nameItem = makeItem(k.installed ? "  " + k.name : k.name);
        nameItem->setIcon(kernelIcon);
        m_table->setItem(row, kColName, nameItem);

        m_table->setItem(row, kColVer,  makeItem(k.version));
        m_table->setItem(row, kColRepo, makeItem(k.repo));
        m_table->setItem(row, kColDesc, makeItem(k.description));

        // Col 4 — status: green "✓ Installed" or muted "⊙ Available"
        auto *statusItem = makeItem(k.installed ? "✓ Installed" : "⊙ Available");
        statusItem->setForeground(k.installed ? QColor("#4ec994") : QColor("#999999"));
        m_table->setItem(row, kColStatus, statusItem);
    }

    m_table->setUpdatesEnabled(true);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void KernelPage::onInstallSelected()
{
    if (m_process->state() != QProcess::NotRunning) return;

    // Collect selected rows that are NOT already installed
    QStringList targets;
    const auto selectedRanges = m_table->selectedItems();
    QSet<int> seenRows;
    for (auto *item : selectedRanges) {
        const int row = item->row();
        if (seenRows.contains(row)) continue;
        seenRows.insert(row);
        if (row < m_kernels.size() && !m_kernels[row].installed)
            targets << m_kernels[row].name;
    }

    if (targets.isEmpty()) {
        emit statusMessage("  Select one or more available kernels to install");
        return;
    }

    // Show dependency-preview dialog
    KernelInstallDialog dlg(targets, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_currentOp = Op::Install;
    setButtonsBusy(true);
    appendOutput(QStringLiteral("Installing: %1").arg(targets.join(", ")));
    emit statusMessage(
        QStringLiteral("  Installing kernel %1...").arg(targets.first()));

    QStringList args = {kPacman, "-S", "--noconfirm"};
    args += targets;
    m_process->start(kPkexec, args);
}

void KernelPage::onRemoveSelected()
{
    if (m_process->state() != QProcess::NotRunning) return;

    QStringList targets;
    QSet<int> seenRows;
    for (auto *item : m_table->selectedItems()) {
        const int row = item->row();
        if (seenRows.contains(row)) continue;
        seenRows.insert(row);
        if (row < m_kernels.size() && m_kernels[row].installed)
            targets << m_kernels[row].name;
    }

    if (targets.isEmpty()) {
        emit statusMessage("  Select one or more installed kernels to remove");
        return;
    }

    if (QMessageBox::warning(this, "Remove Kernel",
            QStringLiteral("Remove kernel(s): %1?\n\n"
                           "Ensure at least one bootable kernel remains.")
                .arg(targets.join(", ")),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::Remove;
    setButtonsBusy(true);
    appendOutput(QStringLiteral("Removing: %1").arg(targets.join(", ")));
    emit statusMessage(
        QStringLiteral("  Removing kernel %1...").arg(targets.first()));

    QStringList args = {kPacman, "-Rns", "--noconfirm"};
    args += targets;
    m_process->start(kPkexec, args);
}

// ── QProcess callbacks ────────────────────────────────────────────────────────

void KernelPage::onReadyRead()
{
    const QString chunk = QString::fromLocal8Bit(m_process->readAll());
    m_outputBuf += chunk;

    // Stream to terminal pane only during user operations
    if (m_currentOp == Op::Install || m_currentOp == Op::Remove) {
        for (const QString &line : chunk.split('\n'))
            if (!line.trimmed().isEmpty())
                appendOutput(line.trimmed());
    }
}

void KernelPage::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const bool    ok  = (status == QProcess::NormalExit) && (exitCode == 0);
    const QString raw = m_outputBuf;
    m_outputBuf.clear();

    const Op finished = m_currentOp;
    m_currentOp = Op::None;

    switch (finished) {

    case Op::QueryInstalled:
        parseInstalledOutput(raw);
        // Phase 2: search sync db for linux packages
        m_currentOp = Op::Search;
        m_process->start(kPacman, {"-Ss", "^linux"});
        return;

    case Op::Search:
        parseSearchOutput(raw);
        break;

    case Op::Install:
        if (ok)
            emit statusMessage("  Kernel installation complete");
        else
            appendOutput(
                QStringLiteral("[vantapm] installation failed (exit %1)").arg(exitCode));
        setButtonsBusy(false);
        loadKernels();
        return;

    case Op::Remove:
        if (ok)
            emit statusMessage("  Kernel removal complete");
        else
            appendOutput(
                QStringLiteral("[vantapm] removal failed (exit %1)").arg(exitCode));
        setButtonsBusy(false);
        loadKernels();
        return;

    default:
        break;
    }

    setButtonsBusy(false);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void KernelPage::appendOutput(const QString &line)
{
    m_outputView->appendPlainText(line);
    m_outputView->verticalScrollBar()->setValue(
        m_outputView->verticalScrollBar()->maximum());
}

void KernelPage::setButtonsBusy(bool busy)
{
    m_btnInstall->setEnabled(!busy);
    m_btnRemove->setEnabled(!busy);
}

void KernelPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";
    const QIcon kernelIcon(prefix + "kernel.png");
    for (int row = 0; row < m_table->rowCount(); ++row)
        if (auto *item = m_table->item(row, kColName))
            item->setIcon(kernelIcon);
}