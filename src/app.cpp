/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 1997 Mario Weilguni <mweilguni@sime.com>
    SPDX-FileCopyrightText: 2002-2004 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2009-2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own
#include "app.h"

// STL
#include <cmath>

// Qt
#include <QIcon>
#include <QLabel>
#include <QPointer>
#include <QStatusBar>
#include <QTimer>

// KF
#include <KActionCollection>
#include <KConfig>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToggleAction>

// KDEGames
#include <KGameHighScoreDialog>
#include <KGameStandardAction>

// LibKmahjongg
#include <KMahjonggConfigDialog>

// KShisen
#include "board.h"
#include "debug.h"
#include "prefs.h"
#include "ui_settings.h"

namespace KShisen
{
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
    KGameStandardAction::gameNew(this, &App::invokeNewGame, actionCollection());
    KGameStandardAction::restart(this, &App::restartGame, actionCollection());
    KGameStandardAction::pause(this, &App::togglePause, actionCollection());
    KGameStandardAction::highscores(this, &App::showHighScores, actionCollection());
    KGameStandardAction::quit(this, &App::close, actionCollection());

    // Move
    KGameStandardAction::undo(this, &App::undo, actionCollection());
    KGameStandardAction::redo(this, &App::redo, actionCollection());
    KGameStandardAction::hint(this, &App::hint, actionCollection());

    auto soundAction = new KToggleAction(i18nc("@option:check", "Play Sounds"), this);
    soundAction->setChecked(Prefs::sounds());
    actionCollection()->addAction(QStringLiteral("sounds"), soundAction);
    connect(soundAction, &KToggleAction::triggered, m_board, &Board::setSoundsEnabled);

    // Settings
    KStandardAction::preferences(this, &App::showSettingsDialog, actionCollection());

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
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Undo))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Redo))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Pause))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Hint))->setEnabled(false);
    } else if (m_board->isPaused()) {
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Undo))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Redo))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Restart))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Pause))->setChecked(true);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Hint))->setEnabled(false);
    } else if (m_board->isStuck()) {
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Pause))->setEnabled(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Hint))->setEnabled(false);
    } else {
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Undo))->setEnabled(m_board->canUndo());
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Redo))->setEnabled(m_board->canRedo());
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Restart))->setEnabled(m_board->canUndo());
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Pause))->setEnabled(true);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Pause))->setChecked(false);
        actionCollection()->action(KGameStandardAction::name(KGameStandardAction::Hint))->setEnabled(true);
    }
}

void App::slotEndOfGame()
{
    if (m_board->tilesLeft() > 0) {
        m_board->setGameStuckEnabled(true);
    } else {
        m_board->setGameOverEnabled(true);
        auto const timeString = i18nc("time string: hh:mm:ss", "%1:%2:%3",
                                      QString::asprintf("%02d", m_board->currentTime() / 3600),
                                      QString::asprintf("%02d", (m_board->currentTime() / 60) % 60),
                                      QString::asprintf("%02d", m_board->currentTime() % 60));
        KGameHighScoreDialog::FieldInfo scoreInfo;
        scoreInfo[KGameHighScoreDialog::Score].setNum(score(m_board->xTiles(), m_board->yTiles(), m_board->currentTime(), m_board->gravityFlag()));
        scoreInfo[KGameHighScoreDialog::Time] = timeString;

        QPointer<KGameHighScoreDialog> scoreDialog = new KGameHighScoreDialog(KGameHighScoreDialog::Name | KGameHighScoreDialog::Time | KGameHighScoreDialog::Score, this);
        scoreDialog->addField(KGameHighScoreDialog::Custom1, i18n("Gravity"), QStringLiteral("gravity"));
        // FIXME: This is bad, because the translated words are stored in the highscores and thus switching the language makes ugly things (schwarzer)
        if (m_board->gravityFlag()) {
            scoreInfo[KGameHighScoreDialog::Custom1] = i18n("Yes");
        } else {
            scoreInfo[KGameHighScoreDialog::Custom1] = i18n("No");
        }
        auto const configGroup = QStringLiteral("%1x%2").arg(m_board->xTiles()).arg(m_board->yTiles());
        scoreDialog->setConfigGroup(qMakePair(QByteArray(configGroup.toUtf8()), configGroup));

        if (m_board->hasCheated()) {
            auto const message = i18n("\nYou could have been in the high scores\nif you did not use Undo or Hint.\nTry without them next time.");
            KMessageBox::information(this, message, i18nc("@title:window", "End of Game"));
        } else {
            if (scoreDialog->addScore(scoreInfo) > 0) {
                auto const message = i18n("Congratulations!\nYou made it into the hall of fame.");
                scoreDialog->setComment(message);
                scoreDialog->exec();
            } else {
                auto const message = i18nc("%1 - time string like hh:mm:ss", "You made it in %1", timeString);
                KMessageBox::information(this, message, i18nc("@title:window", "End of Game"));
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
    //qCDebug(KSHISEN_General) << "Time: " << m_board->currentTime();
    auto const currentTime = m_board->currentTime();
    auto const message = i18n("Your time: %1:%2:%3 %4",
                              QString::asprintf("%02d", currentTime / 3600),
                              QString::asprintf("%02d", (currentTime / 60) % 60),
                              QString::asprintf("%02d", currentTime % 60),
                              m_board->isPaused() ? i18n("(Paused) ") : QString());

    m_gameTimerLabel->setText(message);
}

void App::updateTileDisplay()
{
    auto const numberOfTiles = m_board->tiles();
    m_gameTilesLabel->setText(i18n("Removed: %1/%2 ", numberOfTiles - m_board->tilesLeft(), numberOfTiles));
}

void App::updateCheatDisplay()
{
    m_gameCheatLabel->setVisible(m_board->hasCheated());
}

int App::score(int x, int y, int seconds, bool gravity)
{
    auto const nTiles = static_cast<double>(x * y);
    auto const tilesPerSec = nTiles / static_cast<double>(seconds);

    auto const sizeBonus = std::sqrt(nTiles / 14.0 * 6.0);
    auto const points = tilesPerSec / 0.14 * 100.0;
    auto const gravityBonus = gravity ? 2.0 : 1.0;

    return static_cast<int>(points * sizeBonus * gravityBonus);
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

void App::showHighScores()
{
    KGameHighScoreDialog scoreDialog(KGameHighScoreDialog::Name | KGameHighScoreDialog::Time, this);
    scoreDialog.addField(KGameHighScoreDialog::Custom1, i18n("Gravity"), QStringLiteral("gravity"));
    auto const configGroup = QStringLiteral("%1x%2").arg(m_board->xTiles()).arg(m_board->yTiles());
    scoreDialog.setConfigGroup(qMakePair(QByteArray(configGroup.toUtf8()), configGroup));
    scoreDialog.exec();
}

void App::keyBindings()
{
    KShortcutsDialog::showDialog(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
}

void App::showSettingsDialog()
{
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    //Use the classes exposed by LibKMahjongg for our configuration dialog
    auto dialog = new KMahjonggConfigDialog(this, QStringLiteral("settings"), Prefs::self());
    dialog->addPage(new Settings(nullptr), i18nc("@title:tab", "General"), QStringLiteral("games-config-options"));
    dialog->addTilesetPage();
    dialog->addBackgroundPage();

    connect(dialog, &KMahjonggConfigDialog::settingsChanged, m_board, &Board::loadSettings);
    dialog->show();
}
} // namespace KShisen

#include "moc_app.cpp"

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
