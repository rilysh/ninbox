#!/bin/sh

build() {
    if [ ! "$(command -v cc)" ]; then
	echo error: no C compiler was found.
    fi

    DEFLAGS="-Wall -Wextra -O2 -s"
    if [ "$1" = "--color" ]; then
	FLAGS="$DEFLAGS -DENABLE_COLOR"
    elif [ "$1" = "--boxpath" ]; then
	FLAGS="$DEFLAGS -DBOXPATH=\"$2\""
    elif [ "$1" = "--all" ]; then
	FLAGS="$DEFLAGS -DENABLE_COLOR -DBOXPATH=\"$2\""
    else
	FLAGS=
    fi

    cc $DEFLAGS $FLAGS ninbox.c -o ninbox
}

build "$@"

# TODO: Readme
