#include "TerminalWidget.hpp"
#include "SettingsManager.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QRegularExpression>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QTermWidget(parent)
{
    setHistorySize(8000);
    setScrollBarPosition(QTermWidget::ScrollBarRight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    auto &settings = SettingsManager::instance();
    setColorScheme(settings.terminalColorScheme());

    QFont f = QApplication::font();
    f.setFamily(settings.terminalFont());
    f.setPointSizeF(settings.terminalFontSize());
    setTerminalFont(f);

    // ── Actions ──────────────────────────────────────────────────────────────
    m_actionCopy = new QAction("Copy", this);
    connect(m_actionCopy, &QAction::triggered, this, &TerminalWidget::onCopy);

    m_actionPaste = new QAction("Paste", this);
    connect(m_actionPaste, &QAction::triggered, this, &TerminalWidget::onPaste);

    m_actionZoomIn = new QAction("Zoom In", this);
    m_actionZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(m_actionZoomIn, &QAction::triggered, this, &TerminalWidget::onZoomIn);

    m_actionZoomOut = new QAction("Zoom Out", this);
    m_actionZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(m_actionZoomOut, &QAction::triggered, this, &TerminalWidget::onZoomOut);

    addAction(m_actionZoomIn);
    addAction(m_actionZoomOut);

    connect(this, &QTermWidget::receivedData,           this, &TerminalWidget::onReceivedData);
    connect(this, &QTermWidget::termKeyPressed,         this, &TerminalWidget::onKeyPressed);
    connect(this, &QWidget::customContextMenuRequested, this, &TerminalWidget::execContextMenu);

    // Re-apply font/scheme whenever settings change
    connect(&SettingsManager::instance(), &SettingsManager::settingsChanged,
            this, &TerminalWidget::applySettings);
}

void TerminalWidget::applySettings()
{
    auto &settings = SettingsManager::instance();
    setColorScheme(settings.terminalColorScheme());

    QFont f = QApplication::font();
    f.setFamily(settings.terminalFont());
    f.setPointSizeF(settings.terminalFontSize());
    setTerminalFont(f);
}

// ── Public API ────────────────────────────────────────────────────────────────

void TerminalWidget::execute(const QString &command)
{
    sendText(command + '\r');
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void TerminalWidget::onReceivedData(const QString &data)
{
    static const QRegularExpression pressAnyKey(
        QStringLiteral("\\r?\\n?Press any key to continue"),
        QRegularExpression::CaseInsensitiveOption);

    if (data.contains(pressAnyKey))
        emit pressAnyKeyDetected();
}

void TerminalWidget::onKeyPressed(QKeyEvent *ke)
{
    const bool ctrl = ke->modifiers() & Qt::ControlModifier;
    const int  key  = ke->key();

    if (ctrl && (key == Qt::Key_C || key == Qt::Key_D || key == Qt::Key_Z))
        emit cancelKeyPressed();
}

void TerminalWidget::execContextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(m_actionZoomIn);
    menu.addAction(m_actionZoomOut);
    menu.addSeparator();
    menu.addAction(m_actionCopy);
    m_actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    menu.addAction(m_actionPaste);
    menu.exec(mapToGlobal(pos));
}

void TerminalWidget::onCopy()
{
    QApplication::clipboard()->setText(selectedText());
}

void TerminalWidget::onPaste()
{
    paste(QClipboard::Clipboard);
}

void TerminalWidget::onZoomIn()  { emit zoomIn();  }
void TerminalWidget::onZoomOut() { emit zoomOut(); }

// ── Bracket-paste aware paste ─────────────────────────────────────────────────

void TerminalWidget::paste(QClipboard::Mode mode)
{
    QString text = QApplication::clipboard()->text(mode);
    if (text.isEmpty())
        return;

    text.replace("\r\n", "\n");
    text.replace('\n', '\r');
    text.remove(QRegularExpression("\\r+$"));

    bracketText(text);
    sendText(text);
    QApplication::clipboard()->clear();
}