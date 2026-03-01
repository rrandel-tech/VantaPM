#pragma once

#include "Package.hpp"

#include <QList>
#include <QString>
#include <QStringList>

// PacmanParser
// ------------
// Stateless functions that turn raw pacman output into Package lists.
// No I/O, no QObject — pure transformation, easily unit-tested.

namespace PacmanParser
{
    // Parse `pacman -Ss` output into a flat list of packages.
    // Lines come in pairs:
    //   repo/name version [flags]
    //   <indent> description
    QList<Package> parseSearch(const QString &raw);

    // Parse `pacman -Q` / `pacman -Qe` output.
    // Each line: "name version"  (no repo, no description)
    QList<Package> parseQuery(const QString &raw);

    // Parse `pacman -Qi` (one or many packages) output.
    // Blocks are separated by blank lines; each block is one package.
    // Returns full Package structs with repo and description populated.
    QList<Package> parseQueryInfo(const QString &raw);

    // Parse `pacman -Qu` output.
    // Each line: "name oldver -> newver"
    QStringList parseUpgradable(const QString &raw);

    // Parse a single `pacman -Qi` or `pacman -Si` block.
    Package parseInfo(const QString &raw);
}