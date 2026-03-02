#pragma once

#include <QDialog>
#include <QList>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

#include "Backend/Package.hpp"

// UpdateConfirmDialog
// -------------------
// Modal dialog shown when the user clicks "Update System".
// Displays update type, description, list of packages to update,
// and Start Update / Cancel buttons.
// Emits accepted() if the user confirms.

class UpdateConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateConfirmDialog(const QList<Package> &packages,
                                 QWidget *parent = nullptr);

private:
    void setupUi(const QList<Package> &packages);

    QTableWidget *m_table     = nullptr;
    QPushButton  *m_btnStart  = nullptr;
    QPushButton  *m_btnCancel = nullptr;
};