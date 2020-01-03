#!/bin/bash

BASE_DIR="jni"

MANIFEST_FILE="AndroidManifest.xml"
MANIFEST_TMPL="$MANIFEST_FILE.tmpl"

MAIN_H="$BASE_DIR/src/main.h"

VERSION_SUPER=`grep "#define PROGRAM_VERSION_SUPER" $MAIN_H | awk '{print $3}'`
VERSION_MAJOR=`grep "#define PROGRAM_VERSION_MAJOR" $MAIN_H | awk '{print $3}'`
VERSION_MINOR=`grep "#define PROGRAM_VERSION_MINOR" $MAIN_H | awk '{print $3}'`
VERSION_PATCH=`grep "#define PROGRAM_VERSION_PATCH" $MAIN_H | awk '{print $3}'`
VERSION_EXTRA=`grep "#define PROGRAM_VERSION_EXTRA" $MAIN_H	\
    | awk -F\" '{print $2}'					\
    | tr '[A-Z ]' '[a-z-]'`

UNIQUE_VERSION=`echo "$VERSION_SUPER" | wc -l | awk '{ print $1 }'`
if [ "$UNIQUE_VERSION" != "1" ]; then
    echo "ERROR: program version number ('PROGRAM_VERSION_SUPER') not unique!"
    exit
fi

VERSION_NAME="$VERSION_SUPER.$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"
VERSION_CODE=$(printf '%d%02d%02d%02d'	\
    "$VERSION_SUPER"			\
    "$VERSION_MAJOR"			\
    "$VERSION_MINOR"			\
    "$VERSION_PATCH")

if [ "$VERSION_EXTRA" != "" ]; then
    VERSION_NAME="$VERSION_NAME$VERSION_EXTRA"
fi

# echo "::: VERSION_NAME == '$VERSION_NAME'"
# echo "::: VERSION_CODE == '$VERSION_CODE'"

cat "$MANIFEST_TMPL"					\
    | sed -e "s/__VERSION_NAME__/$VERSION_NAME/"	\
    | sed -e "s/__VERSION_CODE__/$VERSION_CODE/"	\
    > "$MANIFEST_FILE"

exit 0
