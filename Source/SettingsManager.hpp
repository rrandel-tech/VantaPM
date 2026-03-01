#pragma once

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QString>

// SettingsManager
// ---------------
// Singleton that owns all persisted settings and the auto-refresh timer.
// Emits settingsChanged() whenever any value is written so widgets can react.
// Emits autoRefreshTriggered() on each timer tick so pages can re-query.
// Sends desktop notifications via notify-send when notificationsEnabled().

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager &instance();

    // ── Appearance ────────────────────────────────────────────────────────────
    QString theme() const;
    void    setTheme(const QString &value);   // "dark" | "light"

    // ── Features ─────────────────────────────────────────────────────────────
    bool aurEnabled() const;
    void setAurEnabled(bool value);
    bool flatpakEnabled() const;
    void setFlatpakEnabled(bool value);

    // ── General ──────────────────────────────────────────────────────────────
    bool autoRefresh() const;
    void setAutoRefresh(bool value);
    int  autoRefreshInterval() const;         // minutes
    void setAutoRefreshInterval(int value);
    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool value);

    // ── Notifications ─────────────────────────────────────────────────────────
    // Sends a desktop notification via notify-send if notifications are enabled.
    // Does nothing if notify-send is not installed or notifications are off.
    void notify(const QString &summary,
                const QString &body    = {},
                const QString &urgency = QStringLiteral("normal")) const;

signals:
    void settingsChanged();

    // Fired by the auto-refresh timer — connect to page refresh logic
    void autoRefreshTriggered();

private:
    explicit SettingsManager(QObject *parent = nullptr);

    void applyAutoRefresh();   // starts/stops/restarts the timer from current settings

    QSettings m_settings;
    QTimer    m_refreshTimer;
};