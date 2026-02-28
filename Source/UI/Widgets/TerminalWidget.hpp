#pragma once

#include <QClipboard>
#include <qtermwidget6/qtermwidget.h>

class QKeyEvent;

class TerminalWidget : public QTermWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);

    void execute(const QString &command);

    signals:
        void cancelKeyPressed();
    void pressAnyKeyDetected();

private slots:
    void onReceivedData(const QString &data);
    void onKeyPressed(QKeyEvent *ke);
    void execContextMenu(const QPoint &pos);
    void onCopy();
    void onPaste();
    void onZoomIn();
    void onZoomOut();
    void applySettings();   // re-applies font/scheme from SettingsManager

private:
    void paste(QClipboard::Mode mode);

    QAction *m_actionCopy    = nullptr;
    QAction *m_actionPaste   = nullptr;
    QAction *m_actionZoomIn  = nullptr;
    QAction *m_actionZoomOut = nullptr;
};