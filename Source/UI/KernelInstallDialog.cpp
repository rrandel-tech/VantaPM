#include "KernelInstallDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>

static constexpr int kColName = 0;
static constexpr int kColVer  = 1;
static constexpr int kColRepo = 2;
static constexpr int kColDesc = 3;

KernelInstallDialog::KernelInstallDialog(const QStringList &packageNames,
                                         QWidget *parent)
    : QDialog(parent)
    , m_process(new QProcess(this))
    , m_targets(packageNames)
{
    setWindowTitle("Installation Details");
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(true);
    setMinimumSize(760, 520);
    setSizeGripEnabled(false);

    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyRead,
            this, &KernelInstallDialog::onReadyRead);
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &KernelInstallDialog::onProcessFinished);

    setupUi();

    // Disable install until deps resolved
    m_btnInstall->setEnabled(false);
    m_btnInstall->setText("⟳  Resolving...");

    // pacman -Sp --print-format lists all packages that will be pulled in
    QStringList args = {"-Sp", "--print-format", "%n\t%v\t%r\t%d"};
    args += packageNames;
    m_process->start("/usr/bin/pacman", args);
}

void KernelInstallDialog::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto makeDivider = [&]() -> QFrame * {
        auto *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("separator");
        return f;
    };

    // ── Section wrapper helper ────────────────────────────────────────────────
    auto makeSection = [&](QLayout *inner) -> QWidget * {
        auto *w = new QWidget;
        w->setObjectName("updateSection");
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(20, 16, 20, 16);
        l->setSpacing(8);
        l->addLayout(inner);
        return w;
    };

    // ── Header: "Installation Details" ───────────────────────────────────────
    {
        auto *inner = new QVBoxLayout;
        inner->setSpacing(10);

        // Title row
        auto *titleRow = new QHBoxLayout;
        titleRow->setSpacing(8);
        auto *titleIco = new QLabel("⊙");
        titleIco->setObjectName("updateDialogIcon");
        auto *titleLbl = new QLabel("Installation Details");
        titleLbl->setObjectName("updateDialogTitle");
        titleRow->addWidget(titleIco);
        titleRow->addWidget(titleLbl);
        titleRow->addStretch();
        inner->addLayout(titleRow);

        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setObjectName("separator");
        inner->addWidget(sep);

        // "Packages to Install (N)" subheader
        auto *subRow = new QHBoxLayout;
        subRow->setSpacing(8);
        auto *subIco = new QLabel("ⓘ");
        subIco->setObjectName("updateDialogIcon");
        m_countLabel = new QLabel(
            QStringLiteral("Packages to Install (%1)").arg(m_targets.size()));
        m_countLabel->setObjectName("updateDialogSectionHdr");
        subRow->addWidget(subIco);
        subRow->addWidget(m_countLabel);
        subRow->addStretch();
        inner->addLayout(subRow);

        root->addWidget(makeSection(inner));
    }

    root->addWidget(makeDivider());

    // ── Package table ─────────────────────────────────────────────────────────
    {
        auto *tableSection = new QWidget;
        tableSection->setObjectName("updateSection");
        auto *tl = new QVBoxLayout(tableSection);
        tl->setContentsMargins(20, 12, 20, 12);
        tl->setSpacing(0);

        m_table = new QTableWidget(0, 4);
        m_table->setObjectName("updateTable");
        m_table->setSelectionMode(QAbstractItemView::NoSelection);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->setShowGrid(false);
        m_table->setAlternatingRowColors(false);
        m_table->verticalHeader()->setVisible(false);
        m_table->setFrameShape(QFrame::NoFrame);
        m_table->setFocusPolicy(Qt::NoFocus);
        m_table->horizontalHeader()->setObjectName("updateTableHeader");
        m_table->horizontalHeader()->setHighlightSections(false);

        m_table->setHorizontalHeaderItem(kColName, new QTableWidgetItem("Package Name"));
        m_table->setHorizontalHeaderItem(kColVer,  new QTableWidgetItem("Version"));
        m_table->setHorizontalHeaderItem(kColRepo, new QTableWidgetItem("Repository"));
        m_table->setHorizontalHeaderItem(kColDesc, new QTableWidgetItem("Description"));

        m_table->horizontalHeader()->setSectionResizeMode(kColName, QHeaderView::ResizeToContents);
        m_table->horizontalHeader()->setSectionResizeMode(kColVer,  QHeaderView::ResizeToContents);
        m_table->horizontalHeader()->setSectionResizeMode(kColRepo, QHeaderView::ResizeToContents);
        m_table->horizontalHeader()->setSectionResizeMode(kColDesc, QHeaderView::Stretch);
        m_table->setMinimumHeight(200);

        tl->addWidget(m_table);
        root->addWidget(tableSection, 1);
    }

    root->addWidget(makeDivider());

    // ── "Ready to Install" banner ─────────────────────────────────────────────
    {
        auto *inner = new QVBoxLayout;
        inner->setSpacing(6);

        auto *readyRow = new QHBoxLayout;
        readyRow->setSpacing(8);
        auto *readyIco = new QLabel("⊙");
        readyIco->setObjectName("updateDialogIcon");
        auto *readyLbl = new QLabel("Ready to Install");
        readyLbl->setObjectName("updateDialogSectionHdr");
        readyRow->addWidget(readyIco);
        readyRow->addWidget(readyLbl);
        readyRow->addStretch();
        inner->addLayout(readyRow);

        auto *noteRow = new QHBoxLayout;
        noteRow->setSpacing(8);
        auto *noteIco = new QLabel("ⓘ");
        noteIco->setObjectName("updateDialogIcon");
        auto *noteLbl = new QLabel(
            "Click Install to begin. Authentication will be requested via polkit.");
        noteLbl->setObjectName("updateDialogValue");
        noteRow->addWidget(noteIco);
        noteRow->addWidget(noteLbl);
        noteRow->addStretch();
        inner->addLayout(noteRow);

        root->addWidget(makeSection(inner));
    }

    root->addWidget(makeDivider());

    // ── Install / Cancel ──────────────────────────────────────────────────────
    {
        auto *btnRow = new QHBoxLayout;
        btnRow->setContentsMargins(0, 0, 0, 0);
        btnRow->setSpacing(0);

        m_btnInstall = new QPushButton("⬇  Install");
        m_btnInstall->setObjectName("updateBtnStart");
        m_btnInstall->setFixedHeight(52);
        m_btnInstall->setCursor(Qt::PointingHandCursor);

        m_btnCancel = new QPushButton("⊗  Cancel");
        m_btnCancel->setObjectName("updateBtnCancel");
        m_btnCancel->setFixedHeight(52);
        m_btnCancel->setCursor(Qt::PointingHandCursor);

        btnRow->addWidget(m_btnInstall);
        btnRow->addWidget(m_btnCancel);

        auto *btnW = new QWidget;
        btnW->setLayout(btnRow);
        root->addWidget(btnW);
    }

    connect(m_btnInstall, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel,  &QPushButton::clicked, this, &QDialog::reject);
}

void KernelInstallDialog::onReadyRead()
{
    m_buf += QString::fromLocal8Bit(m_process->readAll());
}

void KernelInstallDialog::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const bool ok = (status == QProcess::NormalExit) && (exitCode == 0);

    QStringList resolvedLines;
    for (const QString &line : m_buf.split('\n')) {
        const QString t = line.trimmed();
        // Skip blank lines and URL lines that pacman -Sp emits
        if (t.isEmpty() || t.startsWith("http://") || t.startsWith("https://"))
            continue;
        resolvedLines << t;
    }
    m_buf.clear();

    // Fallback: if resolution failed or returned nothing, show requested pkgs
    if (!ok || resolvedLines.isEmpty()) {
        for (const QString &name : m_targets)
            resolvedLines << (name + "\t\t\t");
    }

    populateTable(resolvedLines);

    m_countLabel->setText(
        QStringLiteral("Packages to Install (%1)").arg(m_table->rowCount()));
    m_btnInstall->setEnabled(true);
    m_btnInstall->setText("⬇  Install");
}

void KernelInstallDialog::populateTable(const QStringList &lines)
{
    m_table->setUpdatesEnabled(false);
    m_table->clearContents();
    m_table->setRowCount(lines.size());

    for (int row = 0; row < lines.size(); ++row) {
        const QStringList cols = lines[row].split('\t');
        m_table->setRowHeight(row, 42);

        auto makeItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text.trimmed());
            it->setFlags(Qt::ItemIsEnabled);
            return it;
        };

        m_table->setItem(row, kColName, makeItem(cols.value(0)));
        m_table->setItem(row, kColVer,  makeItem(cols.value(1)));
        m_table->setItem(row, kColRepo, makeItem(cols.value(2)));
        m_table->setItem(row, kColDesc, makeItem(cols.value(3)));
    }

    m_table->setUpdatesEnabled(true);
}