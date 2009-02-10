/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright (C) 2009  Frederik Schwarzer <schwarzerf@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * @file playernamedlg.cpp
 * @author Frederik Schwarzer <schwarzerf@gmail.com>
 * @brief Class implementation for the player name dialog
 */

#include "playernamedlg.h"
#include "ui_playernamedlgwidget.h"

#include <KDebug>

#include <QWidget>

/**
 * @brief Class to set up the player name dialog
 */
class PlayerNameDlgWidget : public QWidget, public Ui::PlayerNameDlgWidget
{
    public:
        PlayerNameDlgWidget(QWidget *parent = 0) : QWidget(parent)
    {
        setupUi(this);
    }
};


PlayerNameDlg::PlayerNameDlg(QWidget *parent) : KDialog(parent)
{
    setCaption(i18n("Enter your Name"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    ui = new PlayerNameDlgWidget(this);
    setMainWidget( ui );
    showButtonSeparator(true);
    enableButtonOk(false);
    ui->nameLineEdit->setFocus();
    setupActions();
    updateButtons();
}

PlayerNameDlg::~PlayerNameDlg()
{
    delete ui;
}


void PlayerNameDlg::setupActions()
{
    connect(ui->nameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateButtons()));
}


void PlayerNameDlg::setPlayerName(const QString &name)
{
    ui->nameLineEdit->setText(name);
}


QString PlayerNameDlg::playerName() const
{
    return ui->nameLineEdit->text();
}


void PlayerNameDlg::updateButtons()
{
    kDebug() << "updateButtons()";
    enableButtonOk(isReady());
}


bool PlayerNameDlg::isReady() const
{
    return !ui->nameLineEdit->text().trimmed().isEmpty();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
