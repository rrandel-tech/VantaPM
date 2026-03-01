#include "SettingsDialog.hpp"
#include "SettingsManager.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

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

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_btnClose, &QPushButton::clicked, this, &SettingsDialog::onClose);

    // Theme buttons emit themeChanged immediately so MainWindow reacts live
    connect(m_btnDark,  &QPushButton::clicked, this, &SettingsDialog::onThemeToggled);
    connect(m_btnLight, &QPushButton::clicked, this, &SettingsDialog::onThemeToggled);

    // All other controls save on change
    connect(m_chkAur,           &QCheckBox::checkStateChanged, this, &SettingsDialog::saveSettings);
    connect(m_chkFlatpak,       &QCheckBox::checkStateChanged, this, &SettingsDialog::saveSettings);
    connect(m_chkAutoRefresh,   &QCheckBox::checkStateChanged, this, &SettingsDialog::saveSettings);
    connect(m_chkNotifications, &QCheckBox::checkStateChanged, this, &SettingsDialog::saveSettings);
    connect(m_spinInterval,     &QSpinBox::valueChanged,       this, &SettingsDialog::saveSettings);
}

void SettingsDialog::loadSettings()
{
    auto &s = SettingsManager::instance();

    // Block signals while loading so we don't trigger saves or themeChanged
    const QSignalBlocker b1(m_btnDark);
    const QSignalBlocker b2(m_btnLight);
    const QSignalBlocker b3(m_chkAur);
    const QSignalBlocker b4(m_chkFlatpak);
    const QSignalBlocker b5(m_chkAutoRefresh);
    const QSignalBlocker b6(m_spinInterval);
    const QSignalBlocker b7(m_chkNotifications);

    m_btnDark->setChecked(s.theme() == "dark");
    m_btnLight->setChecked(s.theme() == "light");

    m_chkAur->setChecked(s.aurEnabled());
    m_chkFlatpak->setChecked(s.flatpakEnabled());

    m_chkAutoRefresh->setChecked(s.autoRefresh());
    m_spinInterval->setValue(s.autoRefreshInterval());
    m_chkNotifications->setChecked(s.notificationsEnabled());
}

void SettingsDialog::saveSettings()
{
    auto &s = SettingsManager::instance();

    s.setAurEnabled(m_chkAur->isChecked());
    s.setFlatpakEnabled(m_chkFlatpak->isChecked());
    s.setAutoRefresh(m_chkAutoRefresh->isChecked());
    s.setAutoRefreshInterval(m_spinInterval->value());
    s.setNotificationsEnabled(m_chkNotifications->isChecked());
}

void SettingsDialog::onThemeToggled()
{
    const QString newTheme = m_btnDark->isChecked() ? "dark" : "light";
    SettingsManager::instance().setTheme(newTheme);
    emit themeChanged(newTheme);
}

void SettingsDialog::syncTheme(bool isDark)
{
    const QSignalBlocker b1(m_btnDark);
    const QSignalBlocker b2(m_btnLight);
    m_btnDark->setChecked(isDark);
    m_btnLight->setChecked(!isDark);
}

void SettingsDialog::onClose()
{
    accept();
}