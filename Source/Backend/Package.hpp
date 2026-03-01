#pragma once

#include <QString>

struct Package
{
    QString name;
    QString version;
    QString repo;
    QString description;
    bool    installed  = false;
    bool    upgradable = false;
};