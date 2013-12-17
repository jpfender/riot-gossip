#!/bin/sh

WATCHSOURCE=vortrag.tex
MAKEFUNC=makeindex

function makeindex() {
    xelatex vortrag.tex >/dev/null 2>&1
}


while true ; do
    inotifywait -e modify $WATCHSOURCE -qq
    $MAKEFUNC
done
