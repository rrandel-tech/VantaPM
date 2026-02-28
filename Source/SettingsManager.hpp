#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager &instance();

    // Appearance
    QString theme() const;
    void    setTheme(const QString &value);

    // Terminal
    QString terminalFont() const;
    void    setTerminalFont(const QString &value);
    int     terminalFontSize() const;
    void    setTerminalFontSize(int value);
    QString terminalColorScheme() const;
    void    setTerminalColorScheme(const QString &value);

    // Features
    bool aurEnabled() const;
    void setAurEnabled(bool value);
    bool flatpakEnabled() const;
    void setFlatpakEnabled(bool value);

    // General
    bool autoRefresh() const;
    void setAutoRefresh(bool value);
    int  autoRefreshInterval() const;
    void setAutoRefreshInterval(int value);
    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool value);

    signals:
        void settingsChanged();

private:
    explicit SettingsManager(QObject *parent = nullptr);

    QSettings m_settings;
};