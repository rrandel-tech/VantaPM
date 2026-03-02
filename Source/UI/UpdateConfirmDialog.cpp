#include "UpdateConfirmDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QHeaderView>
#include <QScrollArea>

UpdateConfirmDialog::UpdateConfirmDialog(const QList<Package> &packages,
                                         QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Updating System");
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(true);
    setMinimumWidth(600);
    setMinimumHeight(480);
    setSizeGripEnabled(false);

    setupUi(packages);
}

void UpdateConfirmDialog::setupUi(const QList<Package> &packages)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto makeDivider = [&]() {
        auto *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("separator");
        return f;
    };

    // ── Section helper ────────────────────────────────────────────────────────
    auto makeSection = [&](QLayout *inner) -> QWidget * {
        auto *w = new QWidget;
        w->setObjectName("updateSection");
        auto *l = new QVBoxLayout(w);
        l->setContentsMargins(20, 16, 20, 16);
        l->setSpacing(8);
        l->addLayout(inner);
        return w;
    };

    auto makeIconLabel = [&](const QString &icon, const QString &text,
                             const QString &objName = {}) -> QWidget * {
        auto *row = new QHBoxLayout;
        row->setSpacing(6);
        auto *ico = new QLabel(icon);
        ico->setObjectName("updateDialogIcon");
        auto *lbl = new QLabel(text);
        if (!objName.isEmpty()) lbl->setObjectName(objName);
        row->addWidget(ico);
        row->addWidget(lbl);
        row->addStretch();
        auto *w = new QWidget;
        w->setLayout(row);
        return w;
    };

    // ── Section 1: Update info ────────────────────────────────────────────────
    {
        auto *inner = new QVBoxLayout;
        inner->setSpacing(6);

        auto *title = new QLabel("Updating System");
        title->setObjectName("updateDialogTitle");

        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setObjectName("separator");

        inner->addWidget(title);
        inner->addWidget(sep);
        inner->addSpacing(4);

        // Update Type row
        auto *typeHdr = new QLabel("ⓘ Update Type:");
        typeHdr->setObjectName("updateDialogKey");
        inner->addWidget(typeHdr);
        auto *typeVal = new QLabel("System (pacman)");
        typeVal->setObjectName("updateDialogValue");
        typeVal->setContentsMargins(16, 0, 0, 0);
        inner->addWidget(typeVal);

        inner->addSpacing(4);

        // Description row
        auto *descHdr = new QLabel("ⓘ Description:");
        descHdr->setObjectName("updateDialogKey");
        inner->addWidget(descHdr);
        auto *descVal = new QLabel("This will update all system packages using pacman.");
        descVal->setObjectName("updateDialogValue");
        descVal->setContentsMargins(16, 0, 0, 0);
        inner->addWidget(descVal);

        root->addWidget(makeSection(inner));
    }

    root->addWidget(makeDivider());

    // ── Section 2: Package list ───────────────────────────────────────────────
    {
        auto *inner = new QVBoxLayout;
        inner->setSpacing(8);

        auto *hdr = new QLabel(
            QStringLiteral("⊞ Packages to Update (%1)").arg(packages.size()));
        hdr->setObjectName("updateDialogSectionHdr");
        inner->addWidget(hdr);

        // Two-column table: Package Name | New Version
        m_table = new QTableWidget(packages.size(), 2);
        m_table->setObjectName("updateTable");
        m_table->setHorizontalHeaderItem(0, new QTableWidgetItem("Package Name"));
        m_table->setHorizontalHeaderItem(1, new QTableWidgetItem("New Version"));
        m_table->horizontalHeader()->setObjectName("updateTableHeader");
        m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        m_table->verticalHeader()->setVisible(false);
        m_table->setShowGrid(false);
        m_table->setFrameShape(QFrame::NoFrame);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->setSelectionMode(QAbstractItemView::NoSelection);
        m_table->setFocusPolicy(Qt::NoFocus);
        m_table->setAlternatingRowColors(false);
        m_table->setFixedHeight(qMin(packages.size() * 34 + 32, 220));

        for (int i = 0; i < packages.size(); ++i) {
            const Package &pkg = packages[i];
            m_table->setRowHeight(i, 34);

            auto *nameItem = new QTableWidgetItem(pkg.name);
            nameItem->setFlags(Qt::ItemIsEnabled);
            m_table->setItem(i, 0, nameItem);

            auto *verItem = new QTableWidgetItem(
                pkg.newVersion.isEmpty() ? pkg.version : pkg.newVersion);
            verItem->setFlags(Qt::ItemIsEnabled);
            verItem->setForeground(QColor("#4ec994"));
            m_table->setItem(i, 1, verItem);
        }

        inner->addWidget(m_table);
        root->addWidget(makeSection(inner));
    }

    root->addWidget(makeDivider());

    // ── Section 3: Ready banner ───────────────────────────────────────────────
    {
        auto *inner = new QVBoxLayout;
        inner->setSpacing(6);

        auto *readyHdr = new QHBoxLayout;
        readyHdr->setSpacing(6);
        auto *readyIco = new QLabel("⊙");
        readyIco->setObjectName("updateDialogIcon");
        auto *readyTitle = new QLabel("Ready to Update");
        readyTitle->setObjectName("updateDialogSectionHdr");
        readyHdr->addWidget(readyIco);
        readyHdr->addWidget(readyTitle);
        readyHdr->addStretch();

        auto *readyHdrW = new QWidget;
        readyHdrW->setLayout(readyHdr);
        inner->addWidget(readyHdrW);

        auto *readyNote = new QHBoxLayout;
        readyNote->setSpacing(6);
        auto *noteIco = new QLabel("ⓘ");
        noteIco->setObjectName("updateDialogIcon");
        auto *noteLbl = new QLabel("Click Start Update to begin. Authentication will be requested via polkit.");
        noteLbl->setObjectName("updateDialogValue");
        readyNote->addWidget(noteIco);
        readyNote->addWidget(noteLbl);
        readyNote->addStretch();

        auto *readyNoteW = new QWidget;
        readyNoteW->setLayout(readyNote);
        inner->addWidget(readyNoteW);

        root->addWidget(makeSection(inner));
    }

    root->addWidget(makeDivider());

    // ── Buttons ───────────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->setSpacing(0);

    m_btnStart = new QPushButton("⟳  Start Update");
    m_btnStart->setObjectName("updateBtnStart");
    m_btnStart->setFixedHeight(52);
    m_btnStart->setCursor(Qt::PointingHandCursor);

    m_btnCancel = new QPushButton("✕  Cancel");
    m_btnCancel->setObjectName("updateBtnCancel");
    m_btnCancel->setFixedHeight(52);
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    btnRow->addWidget(m_btnStart);
    btnRow->addWidget(m_btnCancel);

    auto *btnWidget = new QWidget;
    btnWidget->setLayout(btnRow);
    root->addWidget(btnWidget);

    root->addStretch();

    connect(m_btnStart,  &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}