#include "PacmanBackend.hpp"
#include "PacmanParser.hpp"

static constexpr const char *kPacman = "/usr/bin/pacman";
static constexpr const char *kPkexec = "/usr/bin/pkexec";

PacmanBackend::PacmanBackend(QObject *parent)
    : QObject(parent)
{
}

// ── Read-only queries ─────────────────────────────────────────────────────────

void PacmanBackend::search(const QString &term)
{
    run({kPacman, "-Ss", term}, false, OutputMode::Search);
}

void PacmanBackend::queryInstalled()
{
    run({kPacman, "-Q"}, false, OutputMode::Query);
}

void PacmanBackend::queryInstalledFull()
{
    // pacman -Qi with no arguments queries all installed packages at once.
    // Output is a series of key:value blocks separated by blank lines.
    run({kPacman, "-Qi"}, false, OutputMode::QueryInfo);
}

void PacmanBackend::queryExplicit()
{
    run({kPacman, "-Qe"}, false, OutputMode::Query);
}

void PacmanBackend::queryUpgradable()
{
    run({kPacman, "-Qu"}, false, OutputMode::Upgradable);
}

void PacmanBackend::infoLocal(const QString &package)
{
    run({kPacman, "-Qi", package}, false, OutputMode::Raw);
}

void PacmanBackend::infoSync(const QString &package)
{
    run({kPacman, "-Si", package}, false, OutputMode::Raw);
}

// ── Write operations ──────────────────────────────────────────────────────────

void PacmanBackend::install(const QStringList &packages)
{
    QStringList argv = {kPacman, "-S", "--noconfirm"};
    argv += packages;
    run(argv, true);
}

void PacmanBackend::remove(const QStringList &packages)
{
    QStringList argv = {kPacman, "-Rns", "--noconfirm"};
    argv += packages;
    run(argv, true);
}

void PacmanBackend::sysUpgrade()
{
    run({kPacman, "-Syu", "--noconfirm"}, true);
}

void PacmanBackend::syncDatabases()
{
    run({kPacman, "-Sy"}, true);
}

// ── State ─────────────────────────────────────────────────────────────────────

bool PacmanBackend::isBusy() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

// ── Private ───────────────────────────────────────────────────────────────────

void PacmanBackend::run(const QStringList &argv, bool withPrivilege,
                        OutputMode mode)
{
    if (isBusy()) {
        emit outputLine(QStringLiteral("[vantapm] busy — operation already running"));
        return;
    }

    delete m_process;
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_outputMode = mode;
    m_lineBuf.clear();
    m_fullBuf.clear();

    connect(m_process, &QProcess::readyRead, this, &PacmanBackend::onReadyRead);
    connect(m_process, &QProcess::finished,  this, &PacmanBackend::onFinished);

    QString     program;
    QStringList args;

    if (withPrivilege) {
        program = kPkexec;
        args    = argv;
    } else {
        program = argv.first();
        args    = argv.mid(1);
    }

    m_process->start(program, args);

    if (!m_process->waitForStarted(3000)) {
        emit startError(QStringLiteral("Failed to start: ")
                        + program + ' ' + args.join(' '));
        delete m_process;
        m_process = nullptr;
    }
}

void PacmanBackend::onReadyRead()
{
    const QString chunk = QString::fromLocal8Bit(m_process->readAll());
    m_fullBuf += chunk;
    m_lineBuf += chunk;

    int nl = -1;
    while ((nl = m_lineBuf.indexOf('\n')) != -1) {
        emit outputLine(m_lineBuf.left(nl).trimmed());
        m_lineBuf.remove(0, nl + 1);
    }
}

void PacmanBackend::onFinished(int exitCode, QProcess::ExitStatus status)
{
    if (!m_lineBuf.isEmpty()) {
        emit outputLine(m_lineBuf.trimmed());
        m_lineBuf.clear();
    }

    const bool ok = (status == QProcess::NormalExit && exitCode == 0);

    if (ok) {
        switch (m_outputMode) {
            case OutputMode::Search:
                emit searchResults(PacmanParser::parseSearch(m_fullBuf));
                break;
            case OutputMode::Query:
                emit queryResults(PacmanParser::parseQuery(m_fullBuf));
                break;
            case OutputMode::QueryInfo:
                emit queryResults(PacmanParser::parseQueryInfo(m_fullBuf));
                break;
            case OutputMode::Upgradable:
                emit upgradableResults(PacmanParser::parseUpgradable(m_fullBuf));
                break;
            case OutputMode::Raw:
                break;
        }
    }

    m_fullBuf.clear();
    emit finished(ok, exitCode);
}