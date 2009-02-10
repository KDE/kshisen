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
* @file playernamedlg.h
* @author Frederik Schwarzer <schwarzerf@gmail.com>
* @brief Class declaration for the player name dialog
*/

#ifndef PLAYERNAMEDLG_H
#define PLAYERNAMEDLG_H

#include <kdialog.h>

class PlayerNameDlgWidget;

/**
* @brief A dialog to get the player's name from the user
*/
class PlayerNameDlg : public KDialog
{
    Q_OBJECT
    public:
        /// Constructor
        explicit PlayerNameDlg(QWidget *parent = 0);
        /// Destructor
        ~PlayerNameDlg();
        /// Inserts the given name into the line edit
        void setPlayerName(const QString &name);
        /// Receives the content of the line edit
        QString playerName() const;
    private:
        /// Sets up the signal-slot connections
        void setupActions();
        /// Returns true if all fields of the dialog are filled
        bool isReady() const;

        PlayerNameDlgWidget *ui; ///< Gives access to the widget's elements

    private slots:
        /// Checks the dialog state and sets the buttons accordingly
        void updateButtons();
};

#endif // PLAYERNAMEDLG_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
