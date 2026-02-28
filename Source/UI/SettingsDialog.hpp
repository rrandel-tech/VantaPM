#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QButtonGroup>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onClose();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // Appearance
    QButtonGroup *m_themeGroup       = nullptr;
    QPushButton  *m_btnDark          = nullptr;
    QPushButton  *m_btnLight         = nullptr;

    // Features
    QCheckBox    *m_chkAur           = nullptr;
    QCheckBox    *m_chkFlatpak       = nullptr;

    // General
    QCheckBox    *m_chkAutoRefresh   = nullptr;
    QSpinBox     *m_spinInterval     = nullptr;
    QCheckBox    *m_chkNotifications = nullptr;

    // Terminal
    QComboBox    *m_comboTermFont    = nullptr;
    QComboBox    *m_comboTermScheme  = nullptr;

    QPushButton  *m_btnClose         = nullptr;
};