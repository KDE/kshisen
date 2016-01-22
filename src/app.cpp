/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2007  Mauricio Piacentini <mauricio@tabuleiro.com>          *
 *   Copyright 2009-2011  Frederik Schwarzer <schwarzer@kde.org>           *
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

// own
#include "app.h"

// STL
#include <cmath>

// Qt
#include <QIcon>
#include <QPointer>
#include <QStatusBar>
#include <QTimer>

// KDE
#include <KActionCollection>
#include <KConfig>
#include <KConfigDialog>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSeparator>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KStandardGuiItem>
#include <KToggleAction>

// KDEGames
#include <KScoreDialog>
#include <KStandardGameAction>

// Kmahjongg
#include <kmahjonggconfigdialog.h>

// KShisen
#include "board.h"
#include "debug.h"
#include "prefs.h"
#include "ui_settings.h"


/**
 * @brief Class holding the settings dialog and its functions
 */
class Settings : public QWidget, public Ui::Settings
{
public:
    explicit Settings(QWidget * parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

App::App(QWidget * parent)
    : KXmlGuiWindow(parent)
    , m_gameTipLabel(nullptr)
    , m_gameTimerLabel(nullptr)
    , m_gameTilesLabel(nullptr)
    , m_gameCheatLabel(nullptr)
    , m_board(nullptr)
{
    m_board = new Board(this);
    m_board->setObjectName(QStringLiteral("board"));

    setCentralWidget(m_board);

    setupStatusBar();
    setupActions();
    setupGUI();

    updateItems();
    updateTileDisplay();
}


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
    KStandardGameAction::gameNew(this, SIGNAL(invokeNewGame()), actionCollection());
    KStandardGameAction::restart(this, SLOT(restartGame()), actionCollection());
    KStandardGameAction::pause(this, SLOT(togglePause()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move
    KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    KStandardGameAction::hint(this, SLOT(hint()), actionCollection());

    KToggleAction * soundAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("speaker")), i18n("Play Sounds"), this);
    soundAction->setChecked(Prefs::sounds());
    actionCollection()->addAction(QStringLiteral("sounds"), soundAction);
    connect(soundAction, &KToggleAction::triggered, m_board, &Board::setSoundsEnabled);

    // Settings
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());

    connect(m_board, &Board::cheatStatusChanged, this, &App::updateCheatDisplay);
    connect(m_board, &Board::changed, this, &App::updateItems);
    connect(m_board, &Board::tilesDoNotMatch, this, &App::notifyTilesDoNotMatch);
    connect(m_board, &Board::invalidMove, this, &App::notifyInvalidMove);
    connect(m_board, &Board::selectATile, this, &App::notifySelectATile);
    connect(m_board, &Board::selectAMatchingTile, this, &App::notifySelectAMatchingTile);
    connect(m_board, &Board::selectAMove, this, &App::notifySelectAMove);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &App::updateTimeDisplay);
    timer->start(1000);

    connect(m_board, &Board::tileCountChanged, this, &App::updateTileDisplay);
    connect(m_board, &Board::endOfGame, this, &App::slotEndOfGame);

    connect(this, &App::invokeNewGame, m_board, &Board::newGame);
    connect(m_board, &Board::newGameStarted, this, &App::newGame);
}

void App::newGame()
{
    setCheatModeEnabled(false);
    setPauseEnabled(false);
    updateItems();
    updateTileDisplay();
}

void App::restartGame()
{
    m_board->setUpdatesEnabled(false);
    while (m_board->canUndo()) {
        m_board->undo();
    }
    m_board->resetRedo();
    m_board->resetTimer();
    setCheatModeEnabled(false);
    m_board->setGameOverEnabled(false);
    m_board->setGameStuckEnabled(false);
    m_board->setUpdatesEnabled(true);
    updateItems();
}

void App::togglePause()
{
    m_board->setPauseEnabled(!m_board->isPaused());
}

void App::setPauseEnabled(bool enabled)
{
    m_board->setPauseEnabled(enabled);
    updateItems();
}

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
    updateTileDisplay();
}

void App::redo()
{
    if (!m_board->canRedo()) {
        return;
    }
    m_board->redo();
    updateItems();
    updateTileDisplay();
}

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

void App::updateItems()
{
    if (m_board->isOver()) {
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Undo)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Redo)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Pause)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Hint)))->setEnabled(false);
    } else if (m_board->isPaused()) {
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Undo)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Redo)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Restart)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Pause)))->setChecked(true);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Hint)))->setEnabled(false);
    } else if (m_board->isStuck()) {
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Pause)))->setEnabled(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Hint)))->setEnabled(false);
    } else {
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Undo)))->setEnabled(m_board->canUndo());
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Redo)))->setEnabled(m_board->canRedo());
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Restart)))->setEnabled(m_board->canUndo());
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Pause)))->setEnabled(true);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Pause)))->setChecked(false);
        actionCollection()->action(QLatin1Literal(KStandardGameAction::name(KStandardGameAction::Hint)))->setEnabled(true);
    }
}

void App::slotEndOfGame()
{
    if (m_board->tilesLeft() > 0) {
        m_board->setGameStuckEnabled(true);
    } else {
        m_board->setGameOverEnabled(true);
        QString const timeString = i18nc("time string: hh:mm:ss", "%1:%2:%3",
                                         QString().sprintf("%02d", m_board->currentTime() / 3600),
                                         QString().sprintf("%02d", (m_board->currentTime() / 60) % 60),
                                         QString().sprintf("%02d", m_board->currentTime() % 60));
        KScoreDialog::FieldInfo scoreInfo;
        scoreInfo[KScoreDialog::Score].setNum(score(m_board->xTiles(), m_board->yTiles(), m_board->currentTime(), m_board->gravityFlag()));
        scoreInfo[KScoreDialog::Time] = timeString;

        QPointer<KScoreDialog> scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Time | KScoreDialog::Score, this);
        scoreDialog->addField(KScoreDialog::Custom1, i18n("Gravity"), QStringLiteral("gravity"));
        // FIXME: This is bad, because the translated words are stored in the highscores and thus switching the language makes ugly things (schwarzer)
        if (m_board->gravityFlag()) {
            scoreInfo[KScoreDialog::Custom1] = i18n("Yes");
        } else {
            scoreInfo[KScoreDialog::Custom1] = i18n("No");
        }
        QString const configGroup = QStringLiteral("%1x%2").arg(m_board->xTiles()).arg(m_board->yTiles());
        scoreDialog->setConfigGroup(qMakePair(QByteArray(configGroup.toUtf8()), configGroup));

        if (m_board->hasCheated()) {
            QString const message = i18n("\nYou could have been in the highscores\nif you did not use Undo or Hint.\nTry without them next time.");
            KMessageBox::information(this, message, i18n("End of Game"));
        } else {
            if (scoreDialog->addScore(scoreInfo) > 0) {
                QString const message = i18n("Congratulations!\nYou made it into the hall of fame.");
                scoreDialog->setComment(message);
                scoreDialog->exec();
            } else {
                QString const message = i18nc("%1 - time string like hh:mm:ss", "You made it in %1", timeString);
                KMessageBox::information(this, message, i18n("End of Game"));
            }
        }
        delete scoreDialog;
    }
    updateItems();
}

void App::updateTimeDisplay()
{
    if (m_board->isStuck() || m_board->isOver()) {
        return;
    }
    //qCDebug(KSHISEN_LOG) << "Time: " << m_board->currentTime();
    int const currentTime = m_board->currentTime();
    QString const message = i18n("Your time: %1:%2:%3 %4",
                                 QString().sprintf("%02d", currentTime / 3600),
                                 QString().sprintf("%02d", (currentTime / 60) % 60),
                                 QString().sprintf("%02d", currentTime % 60),
                                 m_board->isPaused() ? i18n("(Paused) ") : QString());

    m_gameTimerLabel->setText(message);
}

void App::updateTileDisplay()
{
    int const numberOfTiles = m_board->tiles();
    m_gameTilesLabel->setText(i18n("Removed: %1/%2 ", numberOfTiles - m_board->tilesLeft(), numberOfTiles));
}

void App::updateCheatDisplay()
{
    m_gameCheatLabel->setVisible(m_board->hasCheated());
}

int App::score(int x, int y, int seconds, bool gravity) const
{
    double const ntiles = x * y;
    double const tilespersec = ntiles / static_cast<double>(seconds);

    double const sizebonus = std::sqrt(ntiles / static_cast<double>(14.0 * 6.0));
    double const points = tilespersec / 0.14 * 100.0;

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

void App::notifyTilesDoNotMatch()
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
    scoreDialog.addField(KScoreDialog::Custom1, i18n("Gravity"), QStringLiteral("gravity"));
    scoreDialog.exec();
}

void App::keyBindings()
{
    KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
}

void App::showSettingsDialog()
{
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    //Use the classes exposed by LibKmahjongg for our configuration dialog
    KMahjonggConfigDialog * dialog = new KMahjonggConfigDialog(this, QStringLiteral("settings"), Prefs::self());
    dialog->addPage(new Settings(nullptr), i18n("General"), QStringLiteral("games-config-options"));
    dialog->addTilesetPage();
    dialog->addBackgroundPage();

    connect(dialog, &KMahjonggConfigDialog::settingsChanged, m_board, &Board::loadSettings);
    dialog->show();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
