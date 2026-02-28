#include "SettingsDialog.hpp"
#include "SettingsManager.hpp"

#include <QFontDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <qtermwidget6/qtermwidget.h>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setFixedWidth(320);
    setModal(true);

    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 12);
    root->setSpacing(14);

    auto makeSeparator = [&]() {
        auto *f = new QFrame;
        f->setFrameShape(QFrame::HLine);
        f->setObjectName("separator");
        return f;
    };

    auto makeSectionLabel = [&](const QString &text) {
        auto *l = new QLabel(text);
        l->setObjectName("sectionLabel");
        return l;
    };

    // ── Appearance ────────────────────────────────────────────────────────────
    root->addWidget(makeSectionLabel("Appearance"));

    auto *themeRow = new QHBoxLayout;
    themeRow->setSpacing(0);

    m_themeGroup = new QButtonGroup(this);
    m_btnDark    = new QPushButton("Dark");
    m_btnLight   = new QPushButton("Light");

    for (auto *btn : {m_btnDark, m_btnLight}) {
        btn->setCheckable(true);
        btn->setFixedHeight(28);
    }
    m_btnDark->setObjectName("themeBtnLeft");
    m_btnLight->setObjectName("themeBtnRight");

    m_themeGroup->addButton(m_btnDark,  0);
    m_themeGroup->addButton(m_btnLight, 1);

    auto *themeLabel = new QLabel("Theme:");
    themeLabel->setObjectName("filterLabel");
    themeRow->addWidget(themeLabel);
    themeRow->addStretch();
    themeRow->addWidget(m_btnDark);
    themeRow->addWidget(m_btnLight);
    root->addLayout(themeRow);

    root->addWidget(makeSeparator());

    // ── Features ──────────────────────────────────────────────────────────────
    root->addWidget(makeSectionLabel("Features"));

    m_chkAur     = new QCheckBox("Enable AUR Support");
    m_chkFlatpak = new QCheckBox("Enable Flatpak Support");
    root->addWidget(m_chkAur);
    root->addWidget(m_chkFlatpak);

    root->addWidget(makeSeparator());

    // ── General ───────────────────────────────────────────────────────────────
    root->addWidget(makeSectionLabel("General"));

    m_chkAutoRefresh = new QCheckBox("Enable Auto-Refresh");
    root->addWidget(m_chkAutoRefresh);

    auto *intervalRow   = new QHBoxLayout;
    auto *intervalLabel = new QLabel("Auto-Refresh Interval (minutes):");
    intervalLabel->setObjectName("filterLabel");
    m_spinInterval = new QSpinBox;
    m_spinInterval->setRange(1, 1440);
    m_spinInterval->setObjectName("settingsSpin");
    m_spinInterval->setFixedWidth(70);
    intervalRow->addWidget(intervalLabel);
    intervalRow->addStretch();
    intervalRow->addWidget(m_spinInterval);
    root->addLayout(intervalRow);

    m_chkNotifications = new QCheckBox("Enable Notifications");
    root->addWidget(m_chkNotifications);

    root->addWidget(makeSeparator());

    // ── Terminal ──────────────────────────────────────────────────────────────
    root->addWidget(makeSectionLabel("Terminal"));

    auto *fontRow   = new QHBoxLayout;
    auto *fontLabel = new QLabel("Font:");
    fontLabel->setObjectName("filterLabel");
    m_comboTermFont = new QComboBox;
    m_comboTermFont->setObjectName("filterCombo");
    for (const QString &fam : QFontDatabase::families()) {
        if (QFontDatabase::isFixedPitch(fam))
            m_comboTermFont->addItem(fam);
    }
    fontRow->addWidget(fontLabel);
    fontRow->addStretch();
    fontRow->addWidget(m_comboTermFont);
    root->addLayout(fontRow);

    auto *schemeRow   = new QHBoxLayout;
    auto *schemeLabel = new QLabel("Color Scheme:");
    schemeLabel->setObjectName("filterLabel");
    m_comboTermScheme = new QComboBox;
    m_comboTermScheme->setObjectName("filterCombo");
    for (const QString &scheme : QTermWidget::availableColorSchemes())
        m_comboTermScheme->addItem(scheme);
    schemeRow->addWidget(schemeLabel);
    schemeRow->addStretch();
    schemeRow->addWidget(m_comboTermScheme);
    root->addLayout(schemeRow);

    root->addStretch();

    // ── Close button ──────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_btnClose = new QPushButton("Close");
    m_btnClose->setObjectName("btnSecondary");
    m_btnClose->setFixedHeight(32);
    m_btnClose->setFixedWidth(80);
    btnRow->addWidget(m_btnClose);
    root->addLayout(btnRow);

    connect(m_btnClose, &QPushButton::clicked, this, &SettingsDialog::onClose);

    connect(m_btnDark,          &QPushButton::clicked,            this, &SettingsDialog::saveSettings);
    connect(m_btnLight,         &QPushButton::clicked,            this, &SettingsDialog::saveSettings);
    connect(m_chkAur,           &QCheckBox::checkStateChanged,    this, &SettingsDialog::saveSettings);
    connect(m_chkFlatpak,       &QCheckBox::checkStateChanged,    this, &SettingsDialog::saveSettings);
    connect(m_chkAutoRefresh,   &QCheckBox::checkStateChanged,    this, &SettingsDialog::saveSettings);
    connect(m_chkNotifications, &QCheckBox::checkStateChanged,    this, &SettingsDialog::saveSettings);
    connect(m_spinInterval,     &QSpinBox::valueChanged,          this, &SettingsDialog::saveSettings);
    connect(m_comboTermFont,    &QComboBox::currentIndexChanged,  this, &SettingsDialog::saveSettings);
    connect(m_comboTermScheme,  &QComboBox::currentIndexChanged,  this, &SettingsDialog::saveSettings);
}

void SettingsDialog::loadSettings()
{
    auto &s = SettingsManager::instance();

    m_btnDark->setChecked(s.theme() == "dark");
    m_btnLight->setChecked(s.theme() == "light");

    m_chkAur->setChecked(s.aurEnabled());
    m_chkFlatpak->setChecked(s.flatpakEnabled());

    m_chkAutoRefresh->setChecked(s.autoRefresh());
    m_spinInterval->setValue(s.autoRefreshInterval());
    m_chkNotifications->setChecked(s.notificationsEnabled());

    const int fontIdx = m_comboTermFont->findText(s.terminalFont());
    m_comboTermFont->setCurrentIndex(fontIdx >= 0 ? fontIdx : 0);

    const int schemeIdx = m_comboTermScheme->findText(s.terminalColorScheme());
    m_comboTermScheme->setCurrentIndex(schemeIdx >= 0 ? schemeIdx : 0);
}

void SettingsDialog::saveSettings()
{
    auto &s = SettingsManager::instance();

    s.setTheme(m_btnDark->isChecked() ? "dark" : "light");
    s.setAurEnabled(m_chkAur->isChecked());
    s.setFlatpakEnabled(m_chkFlatpak->isChecked());
    s.setAutoRefresh(m_chkAutoRefresh->isChecked());
    s.setAutoRefreshInterval(m_spinInterval->value());
    s.setNotificationsEnabled(m_chkNotifications->isChecked());
    s.setTerminalFont(m_comboTermFont->currentText());
    s.setTerminalColorScheme(m_comboTermScheme->currentText());
}

void SettingsDialog::onClose()
{
    accept();
}