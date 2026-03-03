#include "MaintenancePage.hpp"

#include <QHBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>

// ── Construction ──────────────────────────────────────────────────────────────

MaintenancePage::MaintenancePage(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyRead,
            this, &MaintenancePage::onReadyRead);
    connect(m_process, &QProcess::finished,
            this, [this](int code, QProcess::ExitStatus status) {
        onProcessFinished(code, status);
    });

    setupUi();

    // Populate the orphan panel immediately on construction
    queryOrphans();
}

// ── UI Setup ──────────────────────────────────────────────────────────────────

void MaintenancePage::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Left: scrollable task panel ───────────────────────────────────────────
    auto *taskScroll = new QScrollArea;
    taskScroll->setWidgetResizable(true);
    taskScroll->setFrameShape(QFrame::NoFrame);
    taskScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    taskScroll->setFixedWidth(290);

    auto *taskContainer = new QWidget;
    auto *taskLayout    = new QVBoxLayout(taskContainer);
    taskLayout->setContentsMargins(8, 8, 8, 8);
    taskLayout->setSpacing(0);

    setupTaskPanel(taskLayout);
    taskLayout->addStretch();
    taskScroll->setWidget(taskContainer);

    // ── Vertical separator line ───────────────────────────────────────────────
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setObjectName("separator");
    sep->setMaximumWidth(1);

    // ── Right: log + orphan panel ─────────────────────────────────────────────
    auto *rightWidget = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(16, 0, 0, 0);
    rightLayout->setSpacing(10);

    // Log section header
    {
        auto *hdr = new QHBoxLayout;
        auto *ic  = new QLabel("⊙");
        ic->setObjectName("updateStatusIcon");
        auto *tl  = new QLabel("Maintenance Log");
        tl->setObjectName("sectionLabel");
        hdr->addWidget(ic);
        hdr->addWidget(tl);
        hdr->addStretch();
        rightLayout->addLayout(hdr);
    }

    // Log frame
    auto *logFrame       = new QFrame;
    logFrame->setObjectName("maintenanceLogFrame");
    logFrame->setFrameShape(QFrame::StyledPanel);
    auto *logFrameLayout = new QVBoxLayout(logFrame);
    logFrameLayout->setContentsMargins(0, 0, 0, 0);
    logFrameLayout->setSpacing(0);

    // Placeholder shown before any operation has run
    m_placeholder = new QWidget;
    {
        auto *pl = new QVBoxLayout(m_placeholder);
        pl->setAlignment(Qt::AlignCenter);
        pl->setSpacing(8);

        auto *ic = new QLabel("ⓘ");
        ic->setObjectName("updateEmptyIcon");
        ic->setAlignment(Qt::AlignCenter);

        auto *l1 = new QLabel("Maintenance operations will appear here...");
        l1->setObjectName("emptyLabel");
        l1->setAlignment(Qt::AlignCenter);

        auto *l2 = new QLabel("Select a maintenance task from the left panel to begin.");
        l2->setObjectName("emptyHint");
        l2->setAlignment(Qt::AlignCenter);

        pl->addWidget(ic);
        pl->addWidget(l1);
        pl->addWidget(l2);
    }

    // Actual scrolling log view (hidden until first operation)
    m_logView = new QPlainTextEdit;
    m_logView->setObjectName("terminalView");
    m_logView->setReadOnly(true);
    m_logView->setFrameShape(QFrame::NoFrame);
    m_logView->hide();

    logFrameLayout->addWidget(m_placeholder);
    logFrameLayout->addWidget(m_logView);
    rightLayout->addWidget(logFrame, 1);

    // Orphaned packages panel
    auto *orphanFrame  = new QFrame;
    orphanFrame->setObjectName("maintenanceLogFrame");
    orphanFrame->setFrameShape(QFrame::StyledPanel);
    auto *orphanLayout = new QVBoxLayout(orphanFrame);
    orphanLayout->setContentsMargins(10, 8, 10, 10);
    orphanLayout->setSpacing(6);

    {
        auto *hdr = new QHBoxLayout;
        auto *ic  = new QLabel("☰");
        ic->setObjectName("sectionLabel");
        auto *tl  = new QLabel("Orphaned Packages");
        tl->setObjectName("sectionLabel");
        hdr->addWidget(ic);
        hdr->addWidget(tl);
        hdr->addStretch();
        orphanLayout->addLayout(hdr);
    }

    m_orphanLabel = new QLabel("Scanning for orphaned packages...");
    m_orphanLabel->setObjectName("pkgDesc");
    orphanLayout->addWidget(m_orphanLabel);

    m_orphanList = new QLabel;
    m_orphanList->setObjectName("pkgVersion");
    m_orphanList->setWordWrap(true);
    orphanLayout->addWidget(m_orphanList);

    rightLayout->addWidget(orphanFrame);

    // Assemble root layout
    root->addWidget(taskScroll);
    root->addWidget(sep);
    root->addWidget(rightWidget, 1);
}

void MaintenancePage::setupTaskPanel(QVBoxLayout *layout)
{
    // ── Package Cache Management ──────────────────────────────────────────────
    layout->addWidget(makeSectionHeader("☰", "Package Cache Management"));
    layout->addWidget(makeTaskCard(
        "Clear Package Cache",
        "Remove downloaded package files to free disk space",
        [this]{ onClearPackageCache(); }));
    layout->addWidget(makeTaskCard(
        "Clear All Cache",
        "Remove all cached packages including uninstalled ones",
        [this]{ onClearAllCache(); }));
    layout->addSpacing(12);

    // ── Database Maintenance ──────────────────────────────────────────────────
    layout->addWidget(makeSectionHeader("⟳", "Database Maintenance"));
    layout->addWidget(makeTaskCard(
        "Check Database Integrity",
        "Verify the integrity of the package database",
        [this]{ onCheckDatabaseIntegrity(); }));
    layout->addWidget(makeTaskCard(
        "Backup Database",
        "Create a backup of the package database",
        [this]{ onBackupDatabase(); }));
    layout->addWidget(makeTaskCard(
        "Restore Database",
        "Restore a previous backup of the package database",
        [this]{ onRestoreDatabase(); }));
    layout->addWidget(makeTaskCard(
        "Rank Mirrors",
        "Update mirror list with fastest mirrors using reflector",
        [this]{ onRankMirrors(); }));
    layout->addSpacing(12);

    // ── Configuration Management ──────────────────────────────────────────────
    layout->addWidget(makeSectionHeader("☰", "Configuration Management"));
    layout->addWidget(makeTaskCard(
        "Find .pacnew Files",
        "Locate configuration files that need attention after updates",
        [this]{ onFindPacnewFiles(); }));
    layout->addSpacing(12);

    // ── System Maintenance ────────────────────────────────────────────────────
    layout->addWidget(makeSectionHeader("⟳", "System Maintenance"));
    layout->addWidget(makeTaskCard(
        "Check Integrity",
        "Verify package file integrity",
        [this]{ onCheckIntegrity(); }));
    layout->addWidget(makeTaskCard(
        "Clear Package Lock",
        "Remove stale package manager lock files",
        [this]{ onClearPackageLock(); }));
    layout->addSpacing(12);

    // ── Orphan Packages ───────────────────────────────────────────────────────
    layout->addWidget(makeSectionHeader("☰", "Orphan Packages"));
    layout->addWidget(makeTaskCard(
        "Remove Orphaned Packages",
        "Remove packages no longer required by any other package",
        [this]{ onRemoveOrphans(); }));
}

QWidget *MaintenancePage::makeSectionHeader(const QString &icon, const QString &title)
{
    auto *w = new QWidget;
    auto *l = new QHBoxLayout(w);
    l->setContentsMargins(4, 10, 4, 4);
    l->setSpacing(6);

    auto *iLbl = new QLabel(icon);
    iLbl->setObjectName("sectionLabel");
    auto *tLbl = new QLabel(title);
    tLbl->setObjectName("sectionLabel");

    l->addWidget(iLbl);
    l->addWidget(tLbl);
    l->addStretch();
    return w;
}

QWidget *MaintenancePage::makeTaskCard(const QString         &title,
                                       const QString         &description,
                                       std::function<void()>  callback)
{
    // The button IS the card — full clickable surface, no nested buttons.
    // A transparent overlay widget carries the text layout so we get proper
    // label wrapping without fighting QPushButton's internal text engine.
    auto *btn = new QPushButton;
    btn->setObjectName("maintenanceCardBtn");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(62);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Overlay: non-interactive, sits on top of the button surface
    auto *overlay = new QWidget(btn);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto *ol = new QVBoxLayout(overlay);
    ol->setContentsMargins(10, 8, 10, 8);
    ol->setSpacing(2);

    auto *row = new QHBoxLayout;
    row->setSpacing(6);

    auto *iLbl = new QLabel("☰");
    iLbl->setObjectName("updateStatusIcon");
    iLbl->setFixedWidth(16);

    auto *tLbl = new QLabel(title);
    tLbl->setObjectName("pkgName");

    row->addWidget(iLbl);
    row->addWidget(tLbl);
    row->addStretch();
    ol->addLayout(row);

    auto *dLbl = new QLabel(description);
    dLbl->setObjectName("pkgDesc");
    dLbl->setContentsMargins(22, 0, 0, 0);
    dLbl->setWordWrap(true);
    ol->addWidget(dLbl);

    // Keep overlay sized to button via a lightweight event filter
    class OverlaySizer : public QObject {
        QWidget *m_overlay;
    public:
        OverlaySizer(QWidget *overlay, QObject *parent)
            : QObject(parent), m_overlay(overlay) {}
        bool eventFilter(QObject *, QEvent *e) override {
            if (e->type() == QEvent::Resize)
                if (auto *b = qobject_cast<QWidget *>(parent()))
                    m_overlay->setGeometry(b->rect());
            return false;
        }
    };
    btn->installEventFilter(new OverlaySizer(overlay, btn));

    connect(btn, &QPushButton::clicked, this, [this, callback] {
        if (isBusy()) return;
        callback();
    });
    m_taskButtons.append(btn);

    // Wrap with 4 px bottom margin so cards don't touch each other
    auto *wrapper = new QWidget;
    auto *wl      = new QVBoxLayout(wrapper);
    wl->setContentsMargins(0, 0, 0, 4);
    wl->addWidget(btn);
    return wrapper;
}

// ── Process helpers ───────────────────────────────────────────────────────────

void MaintenancePage::runPrivileged(const QStringList &argv)
{
    m_lineBuf.clear();
    m_process->start("/usr/bin/pkexec", argv);
    if (!m_process->waitForStarted(3000)) {
        appendLog("[error] Failed to launch pkexec.");
        m_currentOp = Op::None;
        setTasksBusy(false);
    }
}

void MaintenancePage::runUnprivileged(const QStringList &argv)
{
    m_lineBuf.clear();
    m_process->start(argv.first(), argv.mid(1));
    if (!m_process->waitForStarted(3000)) {
        appendLog(QStringLiteral("[error] Failed to start: ") + argv.first());
        m_currentOp = Op::None;
        setTasksBusy(false);
    }
}

// ── Task slot implementations ─────────────────────────────────────────────────

void MaintenancePage::onClearPackageCache()
{
    m_currentOp = Op::ClearCache;
    setTasksBusy(true);
    showLog();
    appendLog("Clearing package cache (keeping 1 version per package)...");
    // paccache -rk1: keep 1 version of each installed package
    runPrivileged({"paccache", "-rk1"});
}

void MaintenancePage::onClearAllCache()
{
    if (QMessageBox::warning(this, "Clear All Cache",
            "This removes ALL cached packages, including currently installed versions.\n"
            "You will not be able to downgrade without re-downloading.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::ClearAllCache;
    setTasksBusy(true);
    showLog();
    appendLog("Clearing all package cache...");
    // -ruk0: uninstalled pkgs; -rk0: all remaining
    runPrivileged({"/bin/bash", "-c", "paccache -ruk0 && paccache -rk0"});
}

void MaintenancePage::onCheckDatabaseIntegrity()
{
    m_currentOp = Op::CheckDbIntegrity;
    setTasksBusy(true);
    showLog();
    appendLog("Checking database integrity (pacman --check-db-files)...");
    runUnprivileged({"/usr/bin/pacman", "--check-db-files"});
}

void MaintenancePage::onBackupDatabase()
{
    const QString dest = QFileDialog::getExistingDirectory(
        this, "Select Backup Directory", QDir::homePath());
    if (dest.isEmpty()) return;

    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString file  = dest + "/pacman_db_" + stamp + ".tar.gz";

    m_currentOp = Op::BackupDb;
    setTasksBusy(true);
    showLog();
    appendLog("Backing up pacman database to: " + file);
    // tar needs root to read all of /var/lib/pacman/local reliably
    runPrivileged({"tar", "-czf", file, "/var/lib/pacman/local"});
}

void MaintenancePage::onRestoreDatabase()
{
    const QString file = QFileDialog::getOpenFileName(
        this, "Select Database Backup", QDir::homePath(),
        "Archives (*.tar.gz *.tgz)");
    if (file.isEmpty()) return;

    if (QMessageBox::warning(this, "Restore Database",
            "This will overwrite the current pacman local database.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::RestoreDb;
    setTasksBusy(true);
    showLog();
    appendLog("Restoring database from: " + file);
    runPrivileged({"tar", "-xzf", file, "-C", "/"});
}

void MaintenancePage::onRankMirrors()
{
    m_currentOp = Op::RankMirrors;
    setTasksBusy(true);
    showLog();
    appendLog("Ranking mirrors with reflector — this may take a minute...");
    // --latest 20: consider the 20 most recently synced mirrors
    // --sort rate: sort by download speed
    runPrivileged({"reflector",
                   "--latest", "20",
                   "--sort",   "rate",
                   "--save",   "/etc/pacman.d/mirrorlist"});
}

void MaintenancePage::onFindPacnewFiles()
{
    m_currentOp = Op::FindPacnew;
    setTasksBusy(true);
    showLog();
    appendLog("Searching for .pacnew and .pacsave files in /etc...");
    // find doesn't need root; it skips unreadable dirs and continues
    runUnprivileged({"/usr/bin/find", "/etc",
                     "(", "-name", "*.pacnew",
                     "-o", "-name", "*.pacsave", ")"});
}

void MaintenancePage::onCheckIntegrity()
{
    m_currentOp = Op::CheckIntegrity;
    setTasksBusy(true);
    showLog();
    appendLog("Checking package file integrity (pacman -Qk)...");
    runUnprivileged({"/usr/bin/pacman", "-Qk"});
}

void MaintenancePage::onClearPackageLock()
{
    if (QMessageBox::warning(this, "Clear Package Lock",
            "Only proceed if pacman is NOT currently running.\n"
            "Removing the lock while pacman is active will corrupt the database.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::ClearLock;
    setTasksBusy(true);
    showLog();
    appendLog("Removing /var/lib/pacman/db.lck...");
    runPrivileged({"rm", "-f", "/var/lib/pacman/db.lck"});
}

void MaintenancePage::onRemoveOrphans()
{
    if (QMessageBox::warning(this, "Remove Orphaned Packages",
            "This permanently removes all packages no longer required by any other package.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    m_currentOp = Op::RemoveOrphans;
    setTasksBusy(true);
    showLog();
    appendLog("Removing orphaned packages...");
    // Single privileged shell: list orphans then pass them to pacman -Rns
    runPrivileged({"/bin/bash", "-c",
        "orphans=$(pacman -Qtdq 2>/dev/null) && "
        "[ -n \"$orphans\" ] && "
        "pacman -Rns --noconfirm $orphans || "
        "echo 'No orphaned packages to remove.'"});
}

// ── QProcess callbacks ────────────────────────────────────────────────────────

void MaintenancePage::onReadyRead()
{
    m_lineBuf += QString::fromLocal8Bit(m_process->readAll());

    int nl = -1;
    while ((nl = m_lineBuf.indexOf('\n')) != -1) {
        const QString line = m_lineBuf.left(nl).trimmed();
        m_lineBuf.remove(0, nl + 1);
        if (line.isEmpty()) continue;

        if (m_currentOp == Op::QueryOrphans) {
            // Each line from pacman -Qtdq is one package name
            const QString cur = m_orphanList->text();
            m_orphanList->setText(cur.isEmpty() ? line : cur + ", " + line);
        } else {
            appendLog(line);
        }
    }
}

void MaintenancePage::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    // Flush any remaining partial line
    const QString trailing = m_lineBuf.trimmed();
    m_lineBuf.clear();
    if (!trailing.isEmpty()) {
        if (m_currentOp == Op::QueryOrphans) {
            const QString cur = m_orphanList->text();
            m_orphanList->setText(cur.isEmpty() ? trailing : cur + ", " + trailing);
        } else {
            appendLog(trailing);
        }
    }

    // pacman -Qtdq exits 1 when there are no orphans — treat as success
    const bool ok = (status == QProcess::NormalExit)
                    && (exitCode == 0
                        || (m_currentOp == Op::QueryOrphans && exitCode == 1));

    if (m_currentOp == Op::QueryOrphans) {
        const QString names = m_orphanList->text().trimmed();
        if (names.isEmpty()) {
            m_orphanLabel->setText("No orphaned packages found.");
        } else {
            const int count = names.count(',') + 1;
            m_orphanLabel->setText(
                QStringLiteral("Found %1 orphaned package(s):").arg(count));
            emit statusMessage(
                QStringLiteral("  Found %1 orphaned package(s)").arg(count));
        }
        m_currentOp = Op::None;
        setTasksBusy(false);
        return;
    }

    if (!ok)
        appendLog(QStringLiteral("[vantapm] operation failed (exit %1)").arg(exitCode));
    else
        appendLog("[vantapm] done.");

    const Op finished = m_currentOp;
    m_currentOp = Op::None;
    setTasksBusy(false);

    // After a successful orphan removal, refresh the orphan panel
    if (finished == Op::RemoveOrphans && ok)
        queryOrphans();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void MaintenancePage::queryOrphans()
{
    // pacman -Qtdq: quiet, unrequired, nodeps — one package name per line
    // Non-privileged; exit 1 = no orphans (expected)
    if (isBusy()) return;
    m_currentOp = Op::QueryOrphans;
    m_orphanLabel->setText("Scanning for orphaned packages...");
    m_orphanList->clear();
    m_lineBuf.clear();
    m_process->start("/usr/bin/pacman", {"-Qtdq"});
}

void MaintenancePage::showLog()
{
    m_placeholder->hide();
    m_logView->show();
    m_logView->clear();
}

void MaintenancePage::appendLog(const QString &line)
{
    m_logView->appendPlainText(line);
    m_logView->verticalScrollBar()->setValue(
        m_logView->verticalScrollBar()->maximum());
}

void MaintenancePage::setTasksBusy(bool busy)
{
    for (auto *btn : m_taskButtons)
        btn->setEnabled(!busy);
}

bool MaintenancePage::isBusy() const
{
    return m_process->state() != QProcess::NotRunning;
}

void MaintenancePage::updateIcons(bool /*isDark*/)
{
    // Unicode glyphs used throughout — no pixmap swapping needed.
}