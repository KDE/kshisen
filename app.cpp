/* Yo Emacs, this is -*- C++ -*-
 *******************************************************************
 *******************************************************************
 *
 *
 * KSHISEN
 *
 *
 *******************************************************************
 *
 * A japanese game similar to mahjongg
 *
 *******************************************************************
 *
 * Copyright (C)  1997 by Mario Weilguni <mweilguni@sime.com>
 * Copyright (C) 2002-2004 Dave Corrie  <kde@davecorrie.com>
 * Copyright (C) 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
 *
 *******************************************************************
 *
 * This file is part of the KDE project "KSHISEN"
 *
 * KSHISEN is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KSHISEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KSHISEN; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *******************************************************************
 */

#include "app.h"

#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstandardgameaction.h>
#include <khighscore.h>
#include <kdebug.h>
#include <kshortcutsdialog.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>
#include <kconfigdialog.h>

#include <QTimer>
#include <KLineEdit>

#include <cmath>
#include <kglobal.h>
#include "prefs.h"
#include "ui_settings.h"

#include <kmahjonggconfigdialog.h>

class Settings : public QWidget, public Ui::Settings
{
    public:
        Settings(QWidget *parent)
            : QWidget(parent)
        {
            setupUi(this);
        }
};

App::App(QWidget *parent) : KXmlGuiWindow(parent),
    m_cheat(false)
{
    m_highscoreTable = new KHighscore(this);

    // TODO?
    // Would it make sense long term to have a kconfig update rather then
    // havin both formats supported in the code?
    if( m_highscoreTable->hasTable() ) {
        readHighscore();
    } else {
        readOldHighscore();
    }

    setupStatusBar();
    setupActions();

    m_board = new Board(this);
    m_board->setObjectName( "board" );

    setCentralWidget(m_board);

    setupGUI();

    connect(m_board, SIGNAL(changed()), this, SLOT(enableItems()));
    connect(m_board, SIGNAL(tilesDontMatch()), this, SLOT(notifyTilesDontMatch()));
    connect(m_board, SIGNAL(invalidMove()), this, SLOT(notifyInvalidMove()));
    connect(m_board, SIGNAL(selectATile()), this, SLOT(notifySelectATile()));
    connect(m_board, SIGNAL(selectAMatchingTile()), this, SLOT(notifySelectAMatchingTile()));
    connect(m_board, SIGNAL(selectAMove()), this, SLOT(notifySelectAMove()));

    QTimer *t = new QTimer(this);
    t->start(1000);
    connect(t, SIGNAL(timeout()), this, SLOT(updateScore()));
    connect(m_board, SIGNAL(endOfGame()), this, SLOT(slotEndOfGame()));

    qApp->processEvents();

    updateScore();
    enableItems();
}

void App::setupStatusBar()
{
    m_gameTipLabel= new QLabel(i18n("Select a tile"), statusBar());
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
    KStandardGameAction::pause(this, SLOT(pause()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(hallOfFame()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    // Move
    KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
    KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
    KStandardGameAction::hint(this, SLOT(hint()), actionCollection());
    //new KAction(i18n("Is Game Solvable?"), 0, this,
    //	SLOT(isSolvable()), actionCollection(), "move_solvable");

#ifdef DEBUGGING
    // broken ..
    //(void)new KAction(i18n("&Finish"), 0, board, SLOT(finish()), actionCollection(), "move_finish");
#endif

    // Settings
    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
}

void App::hallOfFame()
{
    showHighscore();
}

void App::newGame()
{
    m_board->newGame();
    resetCheatMode();
    enableItems();
}

void App::restartGame()
{
    m_board->setUpdatesEnabled(false);
    while( m_board->canUndo() ) {
        m_board->undo();
    }
    m_board->setUpdatesEnabled(true);
    m_board->resetRedo();
    m_board->update();
    m_board->resetTimer();
    enableItems();
}

void App::isSolvable()
{
    if( m_board->solvable() ) {
        KMessageBox::information(this, i18n("This game is solvable."));
    } else {
        KMessageBox::information(this, i18n("This game is NOT solvable."));
    }
}

void App::pause()
{
    lockMenus(m_board->pause());
}

void App::undo()
{
    if( m_board->canUndo() ) {
        m_board->undo();
        setCheatMode();
        enableItems();
    }
}

void App::redo()
{
    if( m_board->canRedo() ) {
        m_board->redo();
    }
    enableItems();
}

void App::hint()
{
#ifdef DEBUGGING
    m_board->makeHintMove();
#else
    m_board->showHint();
    setCheatMode();
#endif
    enableItems();
}

void App::lockMenus(bool lock)
{
    // Disable all actions apart from (un)pause, quit and those that are help-related.
    // (Only undo/redo and hint actually *need* to be disabled, but disabling everything
    // provides a good visual hint to the user, that they need to unpause to continue.
    KMenu* help = findChild<KMenu*>("help" );
    QList<QAction*> actions = actionCollection()->actions();
    QList<QAction*>::const_iterator actionIter = actions.constBegin();
    QList<QAction*>::const_iterator actionIterEnd = actions.constEnd();

    while( actionIter != actionIterEnd ) {
        QAction* a = *actionIter;
        if( !a->associatedWidgets().contains(help) ) {
            a->setEnabled(!lock);
        }
        ++actionIter;
    }

    actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setEnabled(true);
    actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Quit))->setEnabled(true);

    enableItems();
}

void App::enableItems()
{
    if( !m_board->isPaused() ) {
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Undo))->setEnabled(m_board->canUndo());
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Redo))->setEnabled(m_board->canRedo());
        actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Restart))->setEnabled(m_board->canUndo());
    }
}

void App::slotEndOfGame()
{
    if( m_board->tilesLeft() > 0 ) {
        KMessageBox::information(this, i18n("No more moves possible!"), i18n("End of Game"));
    } else {
        // create highscore entry
        HighScore hs;
        hs.seconds = m_board->getTimeForGame();
        hs.x = m_board->x_tiles();
        hs.y = m_board->y_tiles();
        hs.gravity = (int)m_board->gravityFlag();

        // check if we made it into Top10
        bool isHighscore = false;
        if( m_highscore.size() < HIGHSCORE_MAX ) {
            isHighscore = true;
        } else if( isBetter(hs, m_highscore[HIGHSCORE_MAX-1]) ) {
            isHighscore = true;
        }

        if( isHighscore && !m_cheat ) {
            hs.name = getPlayerName();
            hs.date = time((time_t*)0);
            int rank = insertHighscore(hs);
            showHighscore(rank);
        } else {
            QString s = i18n("Congratulations! You made it in %1:%2:%3",
                    QString().sprintf("%02d", m_board->getTimeForGame()/3600),
                    QString().sprintf("%02d", (m_board->getTimeForGame() / 60) % 60),
                    QString().sprintf("%02d", m_board->getTimeForGame() % 60));

            if( isHighscore ) { // player would have been in the hisghscores if he did not cheat
                s += '\n' + i18n("You could have been in the higscores if you did not use Undo or Hint. Try without them next time.");
            }

            KMessageBox::information(this, s, i18n("End of Game"));
        }
    }

    resetCheatMode();
    m_board->newGame();
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

void App::updateScore()
{
    int t = m_board->getTimeForGame();
    QString s = i18n(" Your time: %1:%2:%3 %4",
            QString().sprintf("%02d", t / 3600 ),
            QString().sprintf("%02d", (t / 60) % 60 ),
            QString().sprintf("%02d", t % 60 ),
            m_board->isPaused()?i18n("(Paused) "):QString());

    //statusBar()->changeItem(s, SBI_TIME);
    m_gameTimerLabel->setText(s);

    // Number of tiles
    int tl = (m_board->x_tiles() * m_board->y_tiles());
    s = i18n(" Removed: %1/%2 ",
            QString().sprintf("%d", tl - m_board->tilesLeft()),
            QString().sprintf("%d", tl ));

    //statusBar()->changeItem(s, SBI_TILES);
    m_gameTilesLabel->setText(s);
}

void App::setCheatMode()
{
    // set the cheat mode if not set
    if( !m_cheat ) {
        m_cheat = true;
        m_gameCheatLabel->show();
    }
}

void App::resetCheatMode()
{
    // reset cheat mode if set
    if( m_cheat ) {
        m_cheat = false;
        m_gameCheatLabel->hide();
    }
}

QString App::getPlayerName()
{
    KDialog dlg(this);
    dlg.setObjectName( "Hall of Fame" );
    dlg.setButtons( KDialog::Ok );

    QWidget* dummy = new QWidget( &dlg );
    dlg.setMainWidget( dummy );

    QLabel *l1 = new QLabel(i18n("You have made it into the \"Hall Of Fame\". Type in\nyour name so mankind will always remember\nyour cool rating."), dummy);

    QLabel *l2 = new QLabel(i18n("Your name:"), dummy);

    KLineEdit *e = new KLineEdit( dummy );
    e->setMinimumWidth( e->fontMetrics().width( "XXXXXXXXXXXXXXXX" ) );
    e->setText( m_lastPlayerName );
    e->setFocus();

    // create layout
    QHBoxLayout *tl1 = new QHBoxLayout;
    tl1->addWidget(l2);
    tl1->addWidget(e);
    QVBoxLayout *tl = new QVBoxLayout( dummy );
    tl->setMargin( 10 );
    tl->setSpacing( 5 );
    tl->addWidget(l1);
    tl->addLayout(tl1);

    dlg.exec();

    m_lastPlayerName = e->text();

    if( m_lastPlayerName.isEmpty() ) {
        return " ";
    }
    return m_lastPlayerName;
}

int App::getScore(const HighScore &hs)
{
    double ntiles = hs.x*hs.y;
    double tilespersec = ntiles/(double)hs.seconds;

    double sizebonus = std::sqrt(ntiles/(double)(14.0 * 6.0));
    double points = tilespersec / 0.14 * 100.0;

    if( hs.gravity ) {
        return (int)(2.0 * points * sizebonus);
    } else {
        return (int)(points * sizebonus);
    }
}

bool App::isBetter(const HighScore &hs, const HighScore &than)
{
    if( getScore(hs) > getScore(than) ) {
        return true;
    } else {
        return false;
    }
}

int App::insertHighscore(const HighScore &hs)
{
    int i;

    if( m_highscore.size() == 0 ) {
        m_highscore.resize(1);
        m_highscore[0] = hs;
        writeHighscore();
        return 0;
    } else {
        HighScore last = m_highscore[m_highscore.size() - 1];
        if( isBetter(hs, last) || (m_highscore.size() < HIGHSCORE_MAX) ) {
            if( m_highscore.size() == HIGHSCORE_MAX ) {
                m_highscore[HIGHSCORE_MAX - 1] = hs;
            } else {
                m_highscore.resize(m_highscore.size()+1);
                m_highscore[m_highscore.size() - 1] = hs;
            }

            // sort in new entry
            int bestsofar = m_highscore.size() - 1;
            for( i = m_highscore.size() - 1; i > 0; i-- ) {
                if( isBetter(m_highscore[i], m_highscore[i-1]) ) {
                    // swap entries
                    HighScore temp = m_highscore[i-1];
                    m_highscore[i-1] = m_highscore[i];
                    m_highscore[i] = temp;
                    bestsofar = i - 1;
                }
            }

            writeHighscore();
            return bestsofar;
        }
    }
    return -1;
}

void App::readHighscore()
{
    QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
    hi_x = m_highscoreTable->readList("x", HIGHSCORE_MAX);
    hi_y = m_highscoreTable->readList("y", HIGHSCORE_MAX);
    hi_sec = m_highscoreTable->readList("seconds", HIGHSCORE_MAX);
    hi_date = m_highscoreTable->readList("date", HIGHSCORE_MAX);
    hi_grav = m_highscoreTable->readList("gravity", HIGHSCORE_MAX);
    hi_name = m_highscoreTable->readList("name", HIGHSCORE_MAX);

    m_highscore.resize(0);

    for( int i = 0; i < hi_x.count(); i++ ) {
        m_highscore.resize(i+1);

        HighScore hs;

        hs.x = hi_x[i].toInt();
        hs.y = hi_y[i].toInt();
        hs.seconds = hi_sec[i].toInt();
        hs.date = hi_date[i].toInt();
        hs.date = hi_date[i].toInt();
        hs.gravity = hi_grav[i].toInt();
        hs.name = hi_name[i];

        m_highscore[i] = hs;
    }
}

void App::readOldHighscore()
{
    // this is for before-KHighscore-highscores
    int i;
    QString s, e;
    KSharedConfig::Ptr conf = KGlobal::config();

    m_highscore.resize(0);
    i = 0;
    bool eol = false;
    KConfigGroup group = conf->group("Hall of Fame");
    while( (i < (int)HIGHSCORE_MAX) && !eol ) {
        s.sprintf("Highscore_%d", i);
        if( group.hasKey(s) ) {
            e = group.readEntry(s,QString());
            m_highscore.resize(i+1);

            HighScore hs;

            QStringList e = group.readEntry(s, QString()).split(' ');
            int nelem = e.count();
            hs.x = e.at(0).toInt();
            hs.y = e.at(1).toInt();
            hs.seconds = e.at(2).toInt();
            hs.date = e.at(3).toInt();

            if( nelem == 4 ) { // old version <= 1.1
                hs.gravity = 0;
                hs.name = e.at(4);
            } else {
                hs.gravity = e.at(4).toInt();
                hs.name = e.at(5);
            }

            m_highscore[i] = hs;
        } else {
            eol = true;
        }
        i++;
    }

    //	// freshly installed, add my own highscore
    //	if(m_highscore.size() == 0)
    //	{
    //		HighScore hs;
    //		hs.x = 28;
    //		hs.y = 16;
    //		hs.seconds = 367;
    //		hs.name = "Mario";
    //		m_highscore.resize(1);
    //		m_highscore[0] = hs;
    //	}

    // write in new KHighscore format
    writeHighscore();
    // read form KHighscore format
    readHighscore();
}

void App::writeHighscore()
{
    int i;
    QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
    for( i = 0; i < (int)m_highscore.size(); i++ ) {
        HighScore hs = m_highscore[i];
        hi_x.append(QString::number(hs.x));
        hi_y.append(QString::number(hs.y));
        hi_sec.append(QString::number(hs.seconds));
        hi_date.append(QString::number(hs.date));
        hi_grav.append(QString::number(hs.gravity));
        hi_name.append(hs.name);
    }
    m_highscoreTable->writeList("x", hi_x);
    m_highscoreTable->writeList("y", hi_y);
    m_highscoreTable->writeList("seconds", hi_sec);
    m_highscoreTable->writeList("date", hi_date);
    m_highscoreTable->writeList("gravity", hi_grav);
    m_highscoreTable->writeList("name", hi_name);
    m_highscoreTable->writeAndUnlock();
}

void App::showHighscore(int focusitem)
{
    // this may look a little bit confusing...
    KDialog dlg;
    dlg.setObjectName( "hall_Of_fame" );
    dlg.setButtons( KDialog::Close );
    dlg.setWindowTitle(i18n("Hall of Fame"));

    QWidget* dummy = new QWidget(&dlg);
    dlg.setMainWidget(dummy);

    QVBoxLayout *tl = new QVBoxLayout(dummy);
    tl->setMargin( 10 );

    QLabel *l = new QLabel(i18n("Hall of Fame"), dummy);
    QFont f = font();
    f.setPointSize(24);
    f.setBold(true);
    l->setFont(f);
    l->setAlignment(Qt::AlignCenter);
    tl->addWidget(l);

    // insert highscores in a gridlayout
    QGridLayout *table = new QGridLayout;
    tl->setSpacing(5);
    tl->addLayout(table, 1);

    // add a separator line
    KSeparator *sep = new KSeparator(dummy);
    table->addWidget(sep, 1, 0, 1, 5);

    // add titles
    f = font();
    f.setBold(true);
    l = new QLabel(i18n("Rank"), dummy);
    l->setFont(f);
    table->addWidget(l, 0, 0);
    l = new QLabel(i18n("Name"), dummy);
    l->setFont(f);
    table->addWidget(l, 0, 1);
    l = new QLabel(i18n("Time"), dummy);
    l->setFont(f);
    table->addWidget(l, 0, 2);
    l = new QLabel(i18n("Size"), dummy);
    l->setFont(f);
    table->addWidget(l, 0, 3);
    l = new QLabel(i18n("Score"), dummy);
    l->setFont(f);
    table->addWidget(l, 0, 4);

    QString s;
    QLabel *e[10][5];
    signed i, j;

    for( i = 0; i < 10; i++ ) {
        HighScore hs;
        if( i < m_highscore.size() ) {
            hs = m_highscore[i];
        }

        // insert rank
        s.sprintf("%d", i+1);
        e[i][0] = new QLabel(s, dummy);

        // insert name
        if( i < m_highscore.size() ) {
            e[i][1] = new QLabel(hs.name, dummy);
        } else {
            e[i][1] = new QLabel("", dummy);
        }

        // insert time
        QTime ti(0,0,0);
        if( i < m_highscore.size() ) {
            ti = ti.addSecs(hs.seconds);
            s.sprintf("%02d:%02d:%02d", ti.hour(), ti.minute(), ti.second());
            e[i][2] = new QLabel(s, dummy);
        } else {
            e[i][2] = new QLabel("", dummy);
        }

        // insert size
        if( i < m_highscore.size() ) {
            s.sprintf("%d x %d", hs.x, hs.y);
        } else {
            s = "";
        }

        e[i][3] = new QLabel(s, dummy);

        // insert score
        if( i < m_highscore.size() ) {
            s = QString("%1 %2")
                .arg(getScore(hs))
                .arg(hs.gravity ? i18n("(gravity)") : QString(""));
        } else {
            s = "";
        }

        e[i][4] = new QLabel(s, dummy);
        e[i][4]->setAlignment(Qt::AlignRight);
    }

    f = font();
    f.setBold(true);
    f.setItalic(true);
    for( i = 0; i < 10; i++ ) {
        for( j = 0; j < 5; j++ ) {
            if( (int)i == focusitem ) {
                e[i][j]->setFont(f);
            }
            table->addWidget(e[i][j], i+2, j, Qt::AlignCenter);
        }
    }

    dlg.exec();
}

void App::keyBindings()
{
    KShortcutsDialog::configure( actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this );

}

/**
 * Show Settings dialog.
 */
void App::showSettings(){
    if( KConfigDialog::showDialog("settings") ) {
        return;
    }

    //Use the classes exposed by LibKmahjongg for our configuration dialog
    KMahjonggConfigDialog *dialog = new KMahjonggConfigDialog(this, "settings", Prefs::self());
    dialog->addPage(new Settings(0), i18n("General"), "games-config-options");
    dialog->addTilesetPage();
    dialog->addBackgroundPage();
    dialog->setHelp(QString(),"kshisen");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), m_board, SLOT(loadSettings()));
    dialog->show();
}

#include "app.moc"

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
