#include "RepositoryPage.hpp"

#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QScrollBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QScrollArea>
#include <QTemporaryFile>
#include <QDir>

static constexpr const char *kPacmanConf = "/etc/pacman.conf";
static constexpr const char *kPkexec     = "/usr/bin/pkexec";

// ── Column indices ────────────────────────────────────────────────────────────
static constexpr int kColName     = 0;
static constexpr int kColServer   = 1;
static constexpr int kColStatus   = 2;
static constexpr int kColSigLevel = 3;
static constexpr int kColCount    = 4;

// ─────────────────────────────────────────────────────────────────────────────

RepositoryPage::RepositoryPage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyRead,
            this, &RepositoryPage::onReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RepositoryPage::onProcessFinished);

    setupUi();
    loadFromPacmanConf();
}

// ── UI Construction ───────────────────────────────────────────────────────────

void RepositoryPage::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(16);

    // ══════════════════════════════════════════════════════════════════════════
    // LEFT PANEL
    // ══════════════════════════════════════════════════════════════════════════
    auto *leftWidget = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    // ── Header row: "Repositories" + Refresh ─────────────────────────────────
    {
        auto *hdr = new QHBoxLayout;
        hdr->setSpacing(8);

        auto *ico = new QLabel("⊟");
        ico->setObjectName("sectionLabel");

        auto *title = new QLabel("Repositories");
        title->setObjectName("pageTitle");

        m_btnRefresh = new QPushButton("⟳  Refresh");
        m_btnRefresh->setObjectName("btnSecondary");
        m_btnRefresh->setFixedHeight(30);
        m_btnRefresh->setCursor(Qt::PointingHandCursor);

        hdr->addWidget(ico);
        hdr->addWidget(title);
        hdr->addStretch();
        hdr->addWidget(m_btnRefresh);
        leftLayout->addLayout(hdr);
    }

    // ── Repository table ──────────────────────────────────────────────────────
    m_table = new QTableWidget(0, kColCount);
    m_table->setObjectName("packageTable");
    m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setFrameShape(QFrame::StyledPanel);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->horizontalHeader()->setObjectName("pkgHeader");
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setCornerButtonEnabled(false);

    m_table->setHorizontalHeaderItem(kColName,     new QTableWidgetItem("Repository"));
    m_table->setHorizontalHeaderItem(kColServer,   new QTableWidgetItem("Server/Include"));
    m_table->setHorizontalHeaderItem(kColStatus,   new QTableWidgetItem("Status"));
    m_table->setHorizontalHeaderItem(kColSigLevel, new QTableWidgetItem("SigLevel"));

    m_table->horizontalHeader()->setSectionResizeMode(kColName,     QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColServer,   QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(kColStatus,   QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(kColSigLevel, QHeaderView::ResizeToContents);

    leftLayout->addWidget(m_table, 1);

    // ── Footer buttons: Add / Edit / Remove ───────────────────────────────────
    {
        auto *footer = new QHBoxLayout;
        footer->setSpacing(0);

        auto makeFooterBtn = [&](const QString &label,
                                 const QString &obj) -> QPushButton * {
            auto *btn = new QPushButton(label);
            btn->setObjectName(obj);
            btn->setFixedHeight(36);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            return btn;
        };

        m_btnAdd    = makeFooterBtn("+ Add",    "repoFooterBtnAdd");
        m_btnEdit   = makeFooterBtn("✎ Edit",   "repoFooterBtnEdit");
        m_btnRemove = makeFooterBtn("⊟ Remove", "repoFooterBtnRemove");

        m_btnEdit->setEnabled(false);
        m_btnRemove->setEnabled(false);

        footer->addWidget(m_btnAdd);
        footer->addWidget(m_btnEdit);
        footer->addWidget(m_btnRemove);
        leftLayout->addLayout(footer);
    }

    root->addWidget(leftWidget, 1);

    // ── Vertical separator ────────────────────────────────────────────────────
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setObjectName("separator");
    sep->setMaximumWidth(1);
    root->addWidget(sep);

    // ══════════════════════════════════════════════════════════════════════════
    // RIGHT PANEL — detail pane
    // ══════════════════════════════════════════════════════════════════════════
    auto *detailOuter = buildDetailPane();
    detailOuter->setFixedWidth(440);
    root->addWidget(detailOuter);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(m_btnRefresh, &QPushButton::clicked,      this, &RepositoryPage::onRefresh);
    connect(m_btnAdd,     &QPushButton::clicked,      this, &RepositoryPage::onAdd);
    connect(m_btnEdit,    &QPushButton::clicked,      this, &RepositoryPage::onEdit);
    connect(m_btnRemove,  &QPushButton::clicked,      this, &RepositoryPage::onRemove);
    connect(m_table,      &QTableWidget::cellClicked, this, &RepositoryPage::onRowClicked);
}

QWidget *RepositoryPage::buildDetailPane()
{
    auto *pane = new QWidget;
    auto *pl   = new QVBoxLayout(pane);
    pl->setContentsMargins(0, 0, 0, 0);
    pl->setSpacing(0);

    // ── "Repository Details" header ───────────────────────────────────────────
    {
        auto *hdr = new QWidget;
        hdr->setObjectName("repoDetailHeader");
        auto *hl = new QHBoxLayout(hdr);
        hl->setContentsMargins(12, 10, 12, 10);
        hl->setSpacing(8);

        auto *ico = new QLabel("ⓘ");
        ico->setObjectName("updateStatusIcon");
        auto *lbl = new QLabel("Repository Details");
        lbl->setObjectName("sectionLabel");
        lbl->setStyleSheet("font-size:13px; font-weight:600;");

        hl->addWidget(ico);
        hl->addWidget(lbl);
        hl->addStretch();
        pl->addWidget(hdr);
    }

    // Thin divider under header
    auto *hdivider = new QFrame;
    hdivider->setFrameShape(QFrame::HLine);
    hdivider->setObjectName("separator");
    pl->addWidget(hdivider);

    // ── Stacked: placeholder | details ───────────────────────────────────────
    m_detailStack = new QStackedWidget;

    // Index 0 — placeholder
    auto *placeholder = new QWidget;
    {
        auto *el = new QVBoxLayout(placeholder);
        el->setAlignment(Qt::AlignCenter);
        el->setSpacing(8);

        auto *lbl = new QLabel("Select a repository from the list to view details");
        lbl->setObjectName("pkgDesc");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        el->addWidget(lbl);
    }
    m_detailStack->addWidget(placeholder);  // index 0

    // Index 1 — detail content
    auto *detailScroll = new QScrollArea;
    detailScroll->setWidgetResizable(true);
    detailScroll->setFrameShape(QFrame::NoFrame);
    detailScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *detailContent = new QWidget;
    auto *detailLayout  = new QVBoxLayout(detailContent);
    detailLayout->setContentsMargins(12, 12, 12, 12);
    detailLayout->setSpacing(10);

    // Helper — section sub-header
    auto makeSectionHdr = [&](const QString &title) -> QWidget * {
        auto *w = new QWidget;
        auto *l = new QHBoxLayout(w);
        l->setContentsMargins(0, 4, 0, 2);
        l->setSpacing(0);
        auto *lb = new QLabel(title);
        lb->setObjectName("sectionLabel");
        lb->setStyleSheet("font-size:12px; font-weight:600;");
        l->addWidget(lb);
        l->addStretch();
        return w;
    };

    // Helper — key/value row
    auto makeKV = [&](const QString &key, QLabel *&valueOut) -> QWidget * {
        auto *w = new QWidget;
        auto *l = new QHBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(8);

        auto *kLbl = new QLabel(key);
        kLbl->setObjectName("filterLabel");
        kLbl->setFixedWidth(72);

        valueOut = new QLabel("—");
        valueOut->setObjectName("pkgVersion");
        valueOut->setWordWrap(true);
        valueOut->setTextInteractionFlags(Qt::TextSelectableByMouse);

        l->addWidget(kLbl);
        l->addWidget(valueOut, 1);
        return w;
    };

    // Configuration block
    detailLayout->addWidget(makeSectionHdr("Configuration"));
    {
        auto *block = new QWidget;
        block->setObjectName("flatpakDetailBlock");
        auto *bl = new QVBoxLayout(block);
        bl->setContentsMargins(8, 8, 8, 8);
        bl->setSpacing(6);
        bl->addWidget(makeKV("Name:",     m_detailName));
        bl->addWidget(makeKV("Server:",   m_detailServer));
        bl->addWidget(makeKV("Status:",   m_detailStatus));
        bl->addWidget(makeKV("SigLevel:", m_detailSigLevel));
        detailLayout->addWidget(block);
    }

    detailLayout->addStretch();
    detailScroll->setWidget(detailContent);
    m_detailStack->addWidget(detailScroll);  // index 1

    m_detailStack->setCurrentIndex(0);
    pl->addWidget(m_detailStack, 1);

    return pane;
}

// ── pacman.conf parsing ───────────────────────────────────────────────────────

static const QStringList kBuiltinSections = {"options"};

void RepositoryPage::loadFromPacmanConf()
{
    m_repos.clear();

    QFile file(kPacmanConf);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit statusMessage("  [error] Cannot read /etc/pacman.conf");
        return;
    }

    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();

    Repository current;
    bool inRepo = false;

    static const QRegularExpression sectionRe(R"(^\[([^\]]+)\]\s*$)");
    static const QRegularExpression serverRe(R"(^(?:Server|Include)\s*=\s*(.+)$)");
    static const QRegularExpression sigRe(R"(^SigLevel\s*=\s*(.+)$)");

    for (const QString &line : content.split('\n')) {
        // Detect commented-out section header: #[name]
        if (line.trimmed().startsWith('#')) {
            const QString stripped = line.trimmed().mid(1).trimmed();
            const auto csm = sectionRe.match(stripped);
            if (csm.hasMatch()) {
                if (inRepo && !current.name.isEmpty())
                    m_repos.append(current);
                current         = {};
                current.name    = csm.captured(1).trimmed();
                current.enabled = false;
                inRepo = !kBuiltinSections.contains(current.name.toLower());
                continue;
            }
            // Regular comment line — skip for value parsing
            if (inRepo) continue;
        }

        const auto sm = sectionRe.match(line.trimmed());
        if (sm.hasMatch()) {
            if (inRepo && !current.name.isEmpty())
                m_repos.append(current);
            current         = {};
            current.name    = sm.captured(1).trimmed();
            current.enabled = true;
            inRepo = !kBuiltinSections.contains(current.name.toLower());
            continue;
        }

        if (!inRepo) continue;

        const QString trimmed = line.trimmed();
        if (trimmed.startsWith('#')) continue;

        if (current.server.isEmpty()) {
            const auto rm = serverRe.match(trimmed);
            if (rm.hasMatch())
                current.server = rm.captured(1).trimmed();
        }

        if (current.sigLevel.isEmpty()) {
            const auto sl = sigRe.match(trimmed);
            if (sl.hasMatch())
                current.sigLevel = sl.captured(1).trimmed();
        }
    }

    if (inRepo && !current.name.isEmpty())
        m_repos.append(current);

    populateTable(m_repos);
    emit statusMessage(QStringLiteral("  Found %1 repositor%2")
        .arg(m_repos.size())
        .arg(m_repos.size() == 1 ? "y" : "ies"));
}

// ── Table population ──────────────────────────────────────────────────────────

void RepositoryPage::populateTable(const QList<Repository> &repos)
{
    m_table->setUpdatesEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(repos.size());

    for (int row = 0; row < repos.size(); ++row) {
        const auto &repo = repos[row];
        m_table->setRowHeight(row, 40);

        auto makeItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text);
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            return it;
        };

        auto *nameItem = makeItem(repo.name);
        nameItem->setIcon(QIcon(":/icons/light/repository.png"));
        m_table->setItem(row, kColName, nameItem);

        m_table->setItem(row, kColServer,
            makeItem(repo.server.isEmpty() ? "—" : repo.server));

        auto *statusItem = makeItem(repo.enabled ? "✓ Enabled" : "✗ Disabled");
        statusItem->setForeground(repo.enabled ? QColor("#4ec994") : QColor("#999"));
        m_table->setItem(row, kColStatus, statusItem);

        m_table->setItem(row, kColSigLevel,
            makeItem(repo.sigLevel.isEmpty() ? "-" : repo.sigLevel));
    }

    m_table->setUpdatesEnabled(true);

    m_selectedRow = -1;
    m_btnEdit->setEnabled(false);
    m_btnRemove->setEnabled(false);
    showDetailPlaceholder();
}

// ── Detail pane ───────────────────────────────────────────────────────────────

void RepositoryPage::showDetailPlaceholder()
{
    m_detailStack->setCurrentIndex(0);
}

void RepositoryPage::showDetailForRepo(const Repository &repo)
{
    m_detailName->setText(repo.name.isEmpty()     ? "—" : repo.name);
    m_detailServer->setText(repo.server.isEmpty() ? "—" : repo.server);
    m_detailStatus->setText(repo.enabled ? "Enabled" : "Disabled");
    m_detailStatus->setStyleSheet(repo.enabled ? "color:#4ec994;" : "color:#999;");
    m_detailSigLevel->setText(repo.sigLevel.isEmpty() ? "Not set" : repo.sigLevel);
    m_detailStack->setCurrentIndex(1);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void RepositoryPage::onRefresh()
{
    showDetailPlaceholder();
    loadFromPacmanConf();
}

void RepositoryPage::onRowClicked(int row, int /*col*/)
{
    if (row < 0 || row >= m_repos.size()) return;
    m_selectedRow = row;
    m_btnEdit->setEnabled(true);
    m_btnRemove->setEnabled(true);
    showDetailForRepo(m_repos[row]);
}

void RepositoryPage::onAdd()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, "Add Repository", "Repository name:", QLineEdit::Normal, {}, &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    if (name.trimmed().toLower() == "options") {
        QMessageBox::warning(this, "Add Repository",
            "'options' is a reserved section name.");
        return;
    }

    const QString server = QInputDialog::getText(
        this, "Add Repository",
        "Server URL or Include path\n"
        "(e.g. https://mirror.example.com/$repo/os/$arch\n"
        " or /etc/pacman.d/mirrorlist):",
        QLineEdit::Normal, {}, &ok);
    if (!ok || server.trimmed().isEmpty()) return;

    const bool isInclude = server.trimmed().startsWith('/');
    const QString serverLine = isInclude
        ? QStringLiteral("Include = %1").arg(server.trimmed())
        : QStringLiteral("Server = %1").arg(server.trimmed());

    // Write a temp script that appends the block, then run it via pkexec bash
    const QString block = QStringLiteral("\n[%1]\n%2\n")
        .arg(name.trimmed(), serverLine);

    // Use a temp file to avoid any shell-quoting issues with special chars
    m_pendingWriteData = block;
    m_currentOp = Op::Add;

    // Write block to a temp file in /tmp, then pkexec cat >> pacman.conf
    const QString tmpPath = QStringLiteral("/tmp/vantapm_repo_add.tmp");
    QFile tmpFile(tmpPath);
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit statusMessage("  [error] Cannot write temp file");
        m_currentOp = Op::None;
        return;
    }
    QTextStream ts(&tmpFile);
    ts << block;
    tmpFile.close();

    // pkexec /bin/bash -c "cat /tmp/... >> /etc/pacman.conf && rm /tmp/..."
    runPrivileged({"/bin/bash", "-c",
        QStringLiteral("cat %1 >> %2 && rm -f %1")
            .arg(tmpPath, kPacmanConf)});
}

void RepositoryPage::onEdit()
{
    if (m_selectedRow < 0 || m_selectedRow >= m_repos.size()) return;

    const Repository &repo = m_repos[m_selectedRow];

    bool ok = false;
    const QString newServer = QInputDialog::getText(
        this,
        QStringLiteral("Edit Repository — %1").arg(repo.name),
        "Server URL or Include path:",
        QLineEdit::Normal, repo.server, &ok);
    if (!ok || newServer.trimmed().isEmpty()) return;

    const bool isInclude = newServer.trimmed().startsWith('/');
    const QString newLine = isInclude
        ? QStringLiteral("Include = %1").arg(newServer.trimmed())
        : QStringLiteral("Server = %1").arg(newServer.trimmed());

    // Write the replacement line to a temp file, use awk to splice it in.
    // awk: when we're inside the target section, replace first Server/Include.
    const QString tmpPath = QStringLiteral("/tmp/vantapm_repo_edit.tmp");
    QFile tmpFile(tmpPath);
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit statusMessage("  [error] Cannot write temp file");
        return;
    }
    QTextStream ts(&tmpFile);
    ts << newLine;
    tmpFile.close();

    // awk script: track target section, replace first Server/Include line
    const QString awkScript = QStringLiteral(
        "BEGIN{found=0; replaced=0}"
        "/^\\[%1\\]/{found=1; print; next}"
        "/^\\[/{found=0; replaced=0}"
        "found && !replaced && /^(Server|Include)\\s*=/"
        "  {replaced=1; cmd=\"cat %2\"; cmd | getline line; close(cmd); print line; next}"
        "{print}"
    ).arg(repo.name, tmpPath);

    m_currentOp = Op::Edit;
    runPrivileged({"/bin/bash", "-c",
        QStringLiteral("awk '%1' %2 > /tmp/vantapm_pacman_new.conf"
                       " && cp /tmp/vantapm_pacman_new.conf %2"
                       " && rm -f /tmp/vantapm_pacman_new.conf %3")
            .arg(awkScript, kPacmanConf, tmpPath)});
}

void RepositoryPage::onRemove()
{
    if (m_selectedRow < 0 || m_selectedRow >= m_repos.size()) return;

    const Repository &repo = m_repos[m_selectedRow];

    if (QMessageBox::warning(this, "Remove Repository",
            QStringLiteral("Remove [%1] from /etc/pacman.conf?\n"
                           "This cannot be undone.").arg(repo.name),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) != QMessageBox::Yes)
        return;

    // Use awk to delete from [name] up to (not including) the next [section].
    // Single-quoted awk script avoids all C++ escape / shell-quoting issues.
    const QString awkScript = QStringLiteral(
        "BEGIN{skip=0}"
        "/^\\[%1\\]/{skip=1; next}"
        "/^\\[/ && skip {skip=0}"
        "!skip{print}"
    ).arg(repo.name);

    m_currentOp = Op::Remove;
    runPrivileged({"/bin/bash", "-c",
        QStringLiteral("awk '%1' %2 > /tmp/vantapm_pacman_new.conf"
                       " && cp /tmp/vantapm_pacman_new.conf %2"
                       " && rm -f /tmp/vantapm_pacman_new.conf")
            .arg(awkScript, kPacmanConf)});
}

// ── Process helpers ───────────────────────────────────────────────────────────

void RepositoryPage::runPrivileged(const QStringList &argv)
{
    if (isBusy()) return;
    m_outputBuf.clear();
    m_process->start(kPkexec, argv);
    if (!m_process->waitForStarted(3000)) {
        emit statusMessage("  [error] Failed to launch pkexec");
        m_currentOp = Op::None;
    }
}

bool RepositoryPage::isBusy() const
{
    return m_process->state() != QProcess::NotRunning;
}

void RepositoryPage::onReadyRead()
{
    m_outputBuf += QString::fromLocal8Bit(m_process->readAll());
}

void RepositoryPage::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const bool ok = (status == QProcess::NormalExit) && (exitCode == 0);
    m_outputBuf.clear();

    const Op finished = m_currentOp;
    m_currentOp = Op::None;
    m_pendingWriteData.clear();

    switch (finished) {
    case Op::Add:
        emit statusMessage(ok ? "  Repository added" : "  [error] Failed to add repository");
        break;
    case Op::Edit:
        emit statusMessage(ok ? "  Repository updated" : "  [error] Failed to update repository");
        break;
    case Op::Remove:
        emit statusMessage(ok ? "  Repository removed" : "  [error] Failed to remove repository");
        break;
    default:
        break;
    }

    loadFromPacmanConf();
}

// ── updateIcons ───────────────────────────────────────────────────────────────

void RepositoryPage::updateIcons(bool isDark)
{
    const QString prefix = isDark ? ":/icons/light/" : ":/icons/dark/";
    const QIcon repoIcon(prefix + "repository.png");
    for (int row = 0; row < m_table->rowCount(); ++row)
        if (auto *item = m_table->item(row, kColName))
            item->setIcon(repoIcon);
}