#include "SettingsManager.hpp"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("VantaPM", "VantaPM")
{
}

SettingsManager &SettingsManager::instance()
{
    static SettingsManager s_instance;
    return s_instance;
}

// ── Appearance ────────────────────────────────────────────────────────────────

QString SettingsManager::theme() const
{
    return m_settings.value("appearance/theme", "dark").toString();
}
void SettingsManager::setTheme(const QString &value)
{
    m_settings.setValue("appearance/theme", value);
    emit settingsChanged();
}

// ── Terminal ──────────────────────────────────────────────────────────────────

QString SettingsManager::terminalFont() const
{
    return m_settings.value("terminal/font", "Adwaita Mono").toString();
}
void SettingsManager::setTerminalFont(const QString &value)
{
    m_settings.setValue("terminal/font", value);
    emit settingsChanged();
}

int SettingsManager::terminalFontSize() const
{
    return m_settings.value("terminal/fontSize", 11).toInt();
}
void SettingsManager::setTerminalFontSize(int value)
{
    m_settings.setValue("terminal/fontSize", value);
    emit settingsChanged();
}

QString SettingsManager::terminalColorScheme() const
{
    return m_settings.value("terminal/colorScheme", "Linux").toString();
}
void SettingsManager::setTerminalColorScheme(const QString &value)
{
    m_settings.setValue("terminal/colorScheme", value);
    emit settingsChanged();
}

// ── Features ──────────────────────────────────────────────────────────────────

bool SettingsManager::aurEnabled() const
{
    return m_settings.value("features/aur", true).toBool();
}
void SettingsManager::setAurEnabled(bool value)
{
    m_settings.setValue("features/aur", value);
    emit settingsChanged();
}

bool SettingsManager::flatpakEnabled() const
{
    return m_settings.value("features/flatpak", true).toBool();
}
void SettingsManager::setFlatpakEnabled(bool value)
{
    m_settings.setValue("features/flatpak", value);
    emit settingsChanged();
}

// ── General ───────────────────────────────────────────────────────────────────

bool SettingsManager::autoRefresh() const
{
    return m_settings.value("general/autoRefresh", false).toBool();
}
void SettingsManager::setAutoRefresh(bool value)
{
    m_settings.setValue("general/autoRefresh", value);
    emit settingsChanged();
}

int SettingsManager::autoRefreshInterval() const
{
    return m_settings.value("general/autoRefreshInterval", 30).toInt();
}
void SettingsManager::setAutoRefreshInterval(int value)
{
    m_settings.setValue("general/autoRefreshInterval", value);
    emit settingsChanged();
}

bool SettingsManager::notificationsEnabled() const
{
    return m_settings.value("general/notifications", true).toBool();
}
void SettingsManager::setNotificationsEnabled(bool value)
{
    m_settings.setValue("general/notifications", value);
    emit settingsChanged();
}