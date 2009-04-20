#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT src/*.cpp rc.cpp -o $podir/kshisen.pot
rm -f rc.cpp
