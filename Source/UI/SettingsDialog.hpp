#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QButtonGroup>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // Called by MainWindow when the nav-bar theme button is used,
    // so the dialog buttons stay in sync with the current state.
    void syncTheme(bool isDark);

    signals:
        // Emitted when the user switches theme so MainWindow can re-apply immediately
        void themeChanged(const QString &theme);

private slots:
    void onThemeToggled();
    void onClose();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // ── Appearance ────────────────────────────────────────────────────────────
    QButtonGroup *m_themeGroup  = nullptr;
    QPushButton  *m_btnDark     = nullptr;
    QPushButton  *m_btnLight    = nullptr;

    // ── Features ─────────────────────────────────────────────────────────────
    QCheckBox    *m_chkAur      = nullptr;
    QCheckBox    *m_chkFlatpak  = nullptr;

    // ── General ──────────────────────────────────────────────────────────────
    QCheckBox    *m_chkAutoRefresh   = nullptr;
    QSpinBox     *m_spinInterval     = nullptr;
    QCheckBox    *m_chkNotifications = nullptr;

    QPushButton  *m_btnClose    = nullptr;
};