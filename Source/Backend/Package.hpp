#pragma once

#include <QString>

struct Package
{
    QString name;
    QString version;      // installed / current version
    QString newVersion;   // available upgrade version (empty if not upgradable)
    QString repo;
    QString description;
    bool    installed  = false;
    bool    upgradable = false;
};