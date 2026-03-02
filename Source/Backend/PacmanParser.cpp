#include "PacmanParser.hpp"

#include <QRegularExpression>

namespace PacmanParser
{

// ── parseSearch ───────────────────────────────────────────────────────────────

QList<Package> parseSearch(const QString &raw)
{
    QList<Package> results;
    const QStringList lines = raw.split('\n');

    static const QRegularExpression headerRe(
        QStringLiteral("^([^/]+)/([^\\s]+)\\s+([^\\s]+)(.*)$"));

    Package current;
    bool havePending = false;

    for (const QString &line : lines) {
        if (line.isEmpty())
            continue;

        if (!line[0].isSpace()) {
            if (havePending)
                results.append(current);

            current     = {};
            havePending = false;

            const auto m = headerRe.match(line);
            if (!m.hasMatch())
                continue;

            current.repo      = m.captured(1).trimmed();
            current.name      = m.captured(2).trimmed();
            current.version   = m.captured(3).trimmed();
            current.installed = m.captured(4).contains(
                QStringLiteral("[installed]"), Qt::CaseInsensitive);
            havePending = true;
        } else if (havePending) {
            current.description = line.trimmed();
        }
    }

    if (havePending)
        results.append(current);

    return results;
}

// ── parseQuery ────────────────────────────────────────────────────────────────

QList<Package> parseQuery(const QString &raw)
{
    QList<Package> results;

    for (const QString &line : raw.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        const int sp = trimmed.indexOf(' ');
        if (sp == -1)
            continue;

        Package pkg;
        pkg.name      = trimmed.left(sp);
        pkg.version   = trimmed.mid(sp + 1).trimmed();
        pkg.installed = true;
        results.append(pkg);
    }

    return results;
}

// ── parseQueryInfo ────────────────────────────────────────────────────────────

QList<Package> parseQueryInfo(const QString &raw)
{
    QList<Package> results;

    static const QRegularExpression blockSep(QStringLiteral("\\n{2,}"));
    const QStringList blocks = raw.split(blockSep, Qt::SkipEmptyParts);

    for (const QString &block : blocks) {
        if (block.trimmed().isEmpty())
            continue;
        Package pkg = parseInfo(block);
        if (!pkg.name.isEmpty()) {
            pkg.installed = true;
            results.append(pkg);
        }
    }

    return results;
}

// ── parseUpgradableFull ───────────────────────────────────────────────────────
// Parses `pacman -Qu` output.
// Each line: "name currentver -> newver"
// Example:   "linux 6.8.1.arch1-1 -> 6.8.2.arch1-1"

QList<Package> parseUpgradableFull(const QString &raw)
{
    QList<Package> results;

    static const QRegularExpression lineRe(
        QStringLiteral("^(\\S+)\\s+(\\S+)\\s+->\\s+(\\S+)$"));

    for (const QString &line : raw.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        const auto m = lineRe.match(trimmed);
        if (!m.hasMatch())
            continue;

        Package pkg;
        pkg.name       = m.captured(1);
        pkg.version    = m.captured(2);
        pkg.newVersion = m.captured(3);
        pkg.upgradable = true;
        pkg.installed  = true;
        results.append(pkg);
    }

    return results;
}

// ── parseUpgradable (names only, legacy) ─────────────────────────────────────

QStringList parseUpgradable(const QString &raw)
{
    QStringList names;
    for (const QString &line : raw.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;
        const int sp = trimmed.indexOf(' ');
        if (sp != -1)
            names.append(trimmed.left(sp));
    }
    return names;
}

// ── parseInfo ─────────────────────────────────────────────────────────────────

Package parseInfo(const QString &raw)
{
    Package pkg;

    static const QRegularExpression kvRe(
        QStringLiteral("^([^:]+?)\\s*:\\s*(.*)$"));

    for (const QString &line : raw.split('\n')) {
        const auto m = kvRe.match(line);
        if (!m.hasMatch())
            continue;

        const QString key   = m.captured(1).trimmed();
        const QString value = m.captured(2).trimmed();

        if (key == QStringLiteral("Name"))
            pkg.name = value;
        else if (key == QStringLiteral("Version"))
            pkg.version = value;
        else if (key == QStringLiteral("Repository"))
            pkg.repo = value;
        else if (key == QStringLiteral("Description"))
            pkg.description = value;
    }

    return pkg;
}

} // namespace PacmanParser