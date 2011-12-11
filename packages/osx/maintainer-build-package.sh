#!/bin/sh -e
# Copyright 2011 AZEVEDO Filipe

CURRENT_PWD="$PWD"
QMAKE="$1"

if [ -z "$QMAKE" ]; then
  QMAKE="/usr/local/Trolltech/4.7.4-lgpl/bin/qmake"
fi

cd "$CURRENT_PWD/../.."
./configure -system-sqlite -universal -prefix "$PWD/release/webissues.app/Contents/Resources" -qmake "$QMAKE"
make -j 6
make install
cd "$CURRENT_PWD"
./osx-package.sh "$CURRENT_PWD/../../release/webissues" "$QMAKE"