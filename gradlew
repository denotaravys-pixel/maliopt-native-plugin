#!/bin/sh

APP_NAME="Gradle"
APP_BASE_NAME=${0##*/}

MAX_FD=maximum

warn () { echo "$*"; } >&2
die () { echo; echo "$*"; echo; exit 1; } >&2

cygwin=false
msys=false
darwin=false
nonstop=false
case "$( uname )" in
  CYGWIN* )        cygwin=true  ;;
  Darwin* )        darwin=true  ;;
  MSYS* | MINGW* ) msys=true    ;;
  NONSTOP* )       nonstop=true ;;
esac

app_path=$0
while [ -h "$app_path" ]; do
    ls=$( ls -ld "$app_path" )
    link=${ls#*' -> '}
    case $link in
      /*)  app_path=$link ;;
      *)   app_path=${app_path%"${app_path##*/}"}$link ;;
    esac
done
APP_HOME=$( cd "${app_path%"${app_path##*/}"}." && pwd -P ) || exit

CLASSPATH=$APP_HOME/gradle/wrapper/gradle-wrapper.jar

if [ -n "$JAVA_HOME" ]; then
    if [ -x "$JAVA_HOME/jre/sh/java" ]; then
        JAVACMD=$JAVA_HOME/jre/sh/java
    else
        JAVACMD=$JAVA_HOME/bin/java
    fi
    if [ ! -x "$JAVACMD" ]; then
        die "ERROR: JAVA_HOME invalido: $JAVA_HOME"
    fi
else
    JAVACMD=java
    which java >/dev/null 2>&1 || die "ERROR: java nao encontrado no PATH."
fi

if ! "$cygwin" && ! "$darwin" && ! "$nonstop"; then
    case $MAX_FD in
      max*) MAX_FD=$( ulimit -H -n ) || warn "Nao foi possivel consultar limite de file descriptors" ;;
    esac
    case $MAX_FD in
      '' | soft) ;;
      *) ulimit -n "$MAX_FD" || warn "Nao foi possivel definir limite de file descriptors" ;;
    esac
fi

if "$cygwin" || "$msys"; then
    APP_HOME=$( cygpath --path --mixed "$APP_HOME" )
    CLASSPATH=$( cygpath --path --mixed "$CLASSPATH" )
    JAVACMD=$( cygpath --unix "$JAVACMD" )
fi

exec "$JAVACMD" \
    -Xmx64m \
    -Xms64m \
    $JAVA_OPTS \
    $GRADLE_OPTS \
    -classpath "$CLASSPATH" \
    org.gradle.wrapper.GradleWrapperMain \
    "$@"
