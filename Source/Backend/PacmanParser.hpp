#pragma once

#include "Package.hpp"

#include <QList>
#include <QString>
#include <QStringList>

namespace PacmanParser
{
    // Parse `pacman -Ss` output
    QList<Package> parseSearch(const QString &raw);

    // Parse `pacman -Q` / `pacman -Qe` — name+version per line
    QList<Package> parseQuery(const QString &raw);

    // Parse `pacman -Qi` for one or many packages — full key:value blocks
    QList<Package> parseQueryInfo(const QString &raw);

    // Parse `pacman -Qu` — each line: "name oldver -> newver"
    // Returns Package structs with version=oldver, newVersion=newver, upgradable=true
    QList<Package> parseUpgradableFull(const QString &raw);

    // Parse `pacman -Qu` — names only (legacy)
    QStringList parseUpgradable(const QString &raw);

    // Parse a single `pacman -Qi` or `pacman -Si` block
    Package parseInfo(const QString &raw);
}