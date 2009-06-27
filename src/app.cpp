/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2007  Mauricio Piacentini <mauricio@tabuleiro.com>          *
 *   Copyright 2009  Frederik Schwarzer <schwarzerf@gmail.com>             *
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

#include "app.h"
#include "prefs.h"
#include "ui_settings.h"

#include <kmahjonggconfigdialog.h>
#include <kscoredialog.h>
#include <kstandardgameaction.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <kstatusbar.h>

#include <QTimer>

#include <cmath>


/**
 * @brief Class holding the settings dialog and its functions
 */
class Settings : public QWidget, public Ui::Settings
{
public:
    Settings(QWidget *parent)
            : QWidget(parent) {
        setupUi(this);
    }
};

App::App(QWidget *parent)
  : KXmlGuiWindow(parent),
    m_gameTipLabel(0),
    m_gameTimerLabel(0),
    m_gameTilesLabel(0),
    m_gameCheatLabel(0),
    m_board(0)
{
    m_board = new Board(this);
    m_board->setObjectName("board");

    setCentralWidget(m_board);

    setupStatusBar();
    setupActions();
    setupGUI();

    //Does this even make sense here? (schwarzer)
    //qApp->processEvents();

    updateItems();
}


/**
 * There are four areas in the status bar:
 * - game tip
 * - timer
 * - tile count
 * - cheat mode
 */
void App::setupStatusBar()
{
    m_gameTipLabel = new QLabel(i18n("Select a tile"), statusBar());
    statusBar()->addWidget(m_gameTipLabel, 1);

    m_gameTimerLabel = new QLabel(i18n("Time: 0:00:00"), statusBar());
    statusBar()->addWidget(m_gameTimerLabel);

    m_gameTilesLabel = new QLabel(i18n("Removed: 0/0"), statusBar());
    statusBar()->addWidget(m_gameTilesLabel);

    m_gameCheatLabel = new QLabel(i18n("Cheat mode"), statusBar());
    statusBar()->addWidget(m_gameCheatLabel);
    m_gameCheatLabel->hide();
}

void App::setupActions()
{
    // Game
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::restart(this, SLOT(restartGame()), actionCollection());
    KStandardGameAction::pause(this, SLOT(togglePause()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move
    KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    KStandardGameAction::hint(this, SLOT(hint()), actionCollection());

    // Settings
    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());

    connect(m_board, SIGNAL(cheatStatusChanged()), this, SLOT(updateCheatDisplay()));
    connect(m_board, SIGNAL(changed()), this, SLOT(updateItems()));
    connect(m_board, SIGNAL(tilesDontMatch()), this, SLOT(notifyTilesDontMatch()));
    connect(m_board, SIGNAL(invalidMove()), this, SLOT(notifyInvalidMove()));
    connect(m_board, SIGNAL(selectATile()), this, SLOT(notifySelectATile()));
    connect(m_board, SIGNAL(selectAMatchingTile()), this, SLOT(notifySelectAMatchingTile()));
    connect(m_board, SIGNAL(selectAMove()), this, SLOT(notifySelectAMove()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimeDisplay()));
    timer->start(1000);

    connect(m_board, SIGNAL(changed()), this, SLOT(updateTileDisplay()));

    connect(m_board, SIGNAL(endOfGame()), this, SLOT(slotEndOfGame()));

}

/**
 * Flags set in the previously played game should be reset.
 */
void App::newGame()
{
    m_board->newGame();
    setCheatModeEnabled(false);
    setPauseEnabled(false);
    updateItems();
}

/**
 * Currently this is done by undoing all moves done by the user as yet and
 * resetting the move history and the timer.
 * This might change over time. However, just make sure, the user gets his
 * currently played game as if it was started by the New Game action.
 */
void App::restartGame()
{
    m_board->setUpdatesEnabled(false);
    while (m_board->canUndo()) {
        m_board->undo();
    }
    m_board->setUpdatesEnabled(true);
    m_board->resetRedo();
    m_board->resetTimer();
    setCheatModeEnabled(false);
    m_board->setGameOverEnabled(false);
    m_board->setGameStuckEnabled(false);
    m_board->update();
    updateItems();
}


void App::togglePause()
{
    m_board->setPauseEnabled(!m_board->isPaused());
}

/**
 * If the game is paused, do not show the board and disable actions like undo
 * and such.
 */
void App::setPauseEnabled(bool enabled)
{
    m_board->setPauseEnabled(enabled);
    updateItems();
}

/**
 * The Undo action should set the cheat flag, so the user cannot end up in
 * the highscore dialog by making bad decisions. :)
 */
void App::undo()
{
    if (!m_board->canUndo()) {
        return;
    }
    m_board->undo();
    setCheatModeEnabled(true);

    // If the game is stuck (no matching tiles anymore), the player can decide
    // to undo some steps and try a different approach.
    m_board->setGameStuckEnabled(false);

    updateItems();
}

void App::redo()
{
    if (!m_board->canRedo()) {
        return;
    }
    m_board->redo();
    updateItems();
}

/**
 * The Hint action should set the cheat flag, so the user cannot end up in
 * the highscore dialog by having been told what to do. :)
 */
void App::hint()
{
#ifdef DEBUGGING
    m_board->makeHintMove();
#else
    m_board->showHint();
    setCheatModeEnabled(true);
#endif
    updateItems();
}

/**
 * According to the current state of the game (game over, pause ...) some
 * actions might better be disabled. This is the place to do so.
 */
void App::updateItems()
{
    if (m_board->isOver()) {
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Undo))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Redo))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Hint))->setEnabled(false);
    } else if (m_board->isPaused()) {
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Undo))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Redo))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Restart))->setEnabled(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setChecked(true);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Hint))->setEnabled(false);
    } else {
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Undo))->setEnabled(m_board->canUndo());
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Redo))->setEnabled(m_board->canRedo());
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Restart))->setEnabled(m_board->canUndo());
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setEnabled(true);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setChecked(false);
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Hint))->setEnabled(true);
    }
}

void App::slotEndOfGame()
{
    if (m_board->tilesLeft() > 0) {
        m_board->setGameStuckEnabled(true);
        KMessageBox::information(this, i18n("No more moves possible!"), i18n("End of Game"));
    } else {
        m_board->setGameOverEnabled(true);
        QString timeString = i18nc("time string: hh:mm:ss", "%1:%2:%3",
                                    QString().sprintf("%02d", m_board->currentTime() / 3600),
                                    QString().sprintf("%02d", (m_board->currentTime() / 60) % 60),
                                    QString().sprintf("%02d", m_board->currentTime() % 60));
        KScoreDialog::FieldInfo scoreInfo;
        scoreInfo[KScoreDialog::Score].setNum(score(m_board->xTiles(), m_board->yTiles(), m_board->currentTime(), m_board->gravityFlag()));
        scoreInfo[KScoreDialog::Time] = timeString;

        KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time | KScoreDialog::Score, this);
        scoreDialog.addField(KScoreDialog::Custom1, i18n("Gravity"), "gravity");
        // FIXME: This is bad, because the translated words are stored in the highscores and thus switching the language makes ugly things (schwarzer)
        if (m_board->gravityFlag()) {
            scoreInfo[KScoreDialog::Custom1] = i18n("Yes");
        } else {
            scoreInfo[KScoreDialog::Custom1] = i18n("No");
        }
        scoreDialog.setConfigGroup(QString("%1x%2").arg(sizeX[Prefs::size()]).arg(sizeY[Prefs::size()]));

        if (m_board->hasCheated()) {
            QString message = i18n("\nYou could have been in the highscores\nif you did not use Undo or Hint.\nTry without them next time.");
            KMessageBox::information(this, message, i18n("End of Game"));
        } else {
            if (scoreDialog.addScore(scoreInfo)) {
                QString message = i18n("Congratulations!\nYou made it into the hall of fame.");
                scoreDialog.setComment(message);
                scoreDialog.exec();
            } else {
                QString message = i18nc("%1 - time string like hh:mm:ss", "\nYou made it in %1", timeString);
                KMessageBox::information(this, message, i18n("End of Game"));
            }
        }
    }
    updateItems();
}

void App::updateTimeDisplay()
{
    if (m_board->isStuck() || m_board->isOver()) {
        return;
    }
    int currentTime = m_board->currentTime();
    QString message = i18n("Your time: %1:%2:%3 %4",
                            QString().sprintf("%02d", currentTime / 3600),
                            QString().sprintf("%02d", (currentTime / 60) % 60),
                            QString().sprintf("%02d", currentTime % 60),
                            m_board->isPaused() ? i18n("(Paused) ") : QString());

    m_gameTimerLabel->setText(message);
    // temporary hack until I find out why m_board->tilesLeft() in updateTileDisplay() counts the previous state of the board, not the current (schwarzer)
    updateTileDisplay();
}

void App::updateTileDisplay()
{
    kDebug() << "updateTileDisplay()" << m_board->tilesLeft();
    int numberOfTiles = (m_board->xTiles() * m_board->yTiles());
    QString message = i18n("Removed: %1/%2 ",
                    QString().sprintf("%d", numberOfTiles - m_board->tilesLeft()),
                    QString().sprintf("%d", numberOfTiles));

    m_gameTilesLabel->setText(message);
}

void App::updateCheatDisplay()
{
    m_gameCheatLabel->setVisible(m_board->hasCheated());
}

int App::score(int x, int y, int seconds, bool gravity) const
{
    double ntiles = x * y;
    double tilespersec = ntiles / static_cast<double>(seconds);

    double sizebonus = std::sqrt(ntiles / static_cast<double>(14.0 * 6.0));
    double points = tilespersec / 0.14 * 100.0;

    if (gravity) {
        return static_cast<int>(2.0 * points * sizebonus);
    } else {
        return static_cast<int>(points * sizebonus);
    }
}

void App::notifySelectATile()
{
    m_gameTipLabel->setText(i18n("Select a tile"));
}

void App::notifySelectAMatchingTile()
{
    m_gameTipLabel->setText(i18n("Select a matching tile"));
}

void App::notifySelectAMove()
{
    m_gameTipLabel->setText(i18n("Select the move you want by clicking on the blue line"));
}

void App::notifyTilesDontMatch()
{
    m_gameTipLabel->setText(i18n("This tile did not match the one you selected"));
}

void App::notifyInvalidMove()
{
    m_gameTipLabel->setText(i18n("You cannot make this move"));
}

void App::setCheatModeEnabled(bool enabled)
{
    m_board->setCheatModeEnabled(enabled);
    m_gameCheatLabel->setVisible(enabled);
}

void App::showHighscores()
{
    KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
    scoreDialog.addField(KScoreDialog::Custom1, i18n("Gravity"), "gravity");
    scoreDialog.exec();
}

void App::keyBindings()
{
    KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
}

void App::showSettings()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    //Use the classes exposed by LibKmahjongg for our configuration dialog
    KMahjonggConfigDialog *dialog = new KMahjonggConfigDialog(this, "settings", Prefs::self());
    dialog->addPage(new Settings(0), i18n("General"), "games-config-options");
    dialog->addTilesetPage();
    dialog->addBackgroundPage();
    dialog->setHelp(QString(), "kshisen");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), m_board, SLOT(loadSettings()));
    dialog->show();
}

#include "app.moc"

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
