#include "SettingsManager.hpp"

#include <QProcess>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("VantaPM", "VantaPM")
{
    m_refreshTimer.setSingleShot(false);
    connect(&m_refreshTimer, &QTimer::timeout,
            this, &SettingsManager::autoRefreshTriggered);

    applyAutoRefresh();
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
    if (theme() == value)
        return;
    m_settings.setValue("appearance/theme", value);
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
    applyAutoRefresh();
    emit settingsChanged();
}

int SettingsManager::autoRefreshInterval() const
{
    return m_settings.value("general/autoRefreshInterval", 30).toInt();
}

void SettingsManager::setAutoRefreshInterval(int value)
{
    m_settings.setValue("general/autoRefreshInterval", value);
    applyAutoRefresh();
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

// ── Notifications ─────────────────────────────────────────────────────────────

void SettingsManager::notify(const QString &summary,
                             const QString &body,
                             const QString &urgency) const
{
    if (!notificationsEnabled())
        return;

    // notify-send is part of libnotify — standard on most Linux desktops.
    // We fire-and-forget via QProcess::startDetached; no need to track the PID.
    QStringList args;
    args << QStringLiteral("--app-name=VantaPM")
         << QStringLiteral("--urgency=") + urgency
         << summary;

    if (!body.isEmpty())
        args << body;

    QProcess::startDetached(QStringLiteral("/usr/bin/notify-send"), args);
}

// ── Private ───────────────────────────────────────────────────────────────────

void SettingsManager::applyAutoRefresh()
{
    m_refreshTimer.stop();
    if (autoRefresh()) {
        const int ms = autoRefreshInterval() * 60 * 1000;
        m_refreshTimer.setInterval(ms);
        m_refreshTimer.start();
    }
}