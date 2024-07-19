#!/bin/bash
APP=$(which $1)
ARGS=${@:2}

export LIBGL_DEEPBIND=0
export LD_LIBRARY_PATH="/usr/lib/gl4es:/usr/lib/libmali/$XDG_SESSION_TYPE:$LD_LIBRARY_PATH"

if [ "$XDG_SESSION_TYPE" = "x11" ]; then
    export LD_PRELOAD="/usr/lib/libdri2to3.so:$LD_PRELOAD"
fi

echo "Running ${APP} with libmali"
exec $APP $ARGS
