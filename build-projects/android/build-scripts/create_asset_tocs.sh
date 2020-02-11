#!/bin/bash

ASSETS_PATH="$1"

TOC_BASENAME=".toc"
TOC_FILENAME="$ASSETS_PATH/$TOC_BASENAME"

if [ "$ASSETS_PATH" = "" ]; then
    echo "Usage: $0 <assets directory>"

    exit 5
fi

if [ ! -d "$ASSETS_PATH" ]; then
    echo "ERROR: cannot find directory '$ASSETS_PATH'!"

    exit 10
fi

ls -1Ap "$ASSETS_PATH" | grep -v "^$TOC_BASENAME$" > "$TOC_FILENAME"

for i in "$ASSETS_PATH"/*; do
    if [ -d "$i" ]; then
	$0 "$i"
    fi
done

exit 0
