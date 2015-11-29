/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 2010  Frederik Schwarzer <schwarzer@kde.org>                *
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

// Version numbers are of the form: MAJOR.MINOR.MICRO
// The MAJOR number should only be incremented when really major changes occur.
// The MINOR number is increased to a release if new features were added.
// The MICRO version is increased for every bugfix release.
//
// The MICRO version number is only used in release branches. Trunk/Master uses
// the first two numbers of the current release with a '+' sign to indicate the
// development state.
//
// In master a commit number (see commit.h) indicates the sub version. The
// commit number is increased absolutely and not reset on releases.
//
// Example: Version "1.8.3" is the current stable version, so the master
//          version number is "1.8+" with the commit number "11" resulting
//          in the version info "1.8+ #11".
//          With every substantial change in the code the commit number
//          is increased.
//          With no new features but two changes at release time:
//            - stable version is "1.8.4"
//            - master version is "1.8+ #13"
//          With new features and two changes at release time:
//            - stable version is "1.9"
//            - master version is "1.9+ #13"

#ifndef KSHISEN_VERSION
#define KSHISEN_VERSION "1.9+"
#endif // KSHISEN_VERSION
