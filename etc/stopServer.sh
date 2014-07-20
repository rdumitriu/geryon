#!/bin/bash

#See how are we called
PRG="$0"

if [ -z "$GERYON_HOME" ]; then
  while [ -h "$PRG" ]; do
    ls=`ls -ld "$PRG"`
    link=`expr "$ls" : '.*-> \(.*\)$'`
    if expr "$link" : '/.*' > /dev/null; then
      PRG="$link"
    else
      PRG=`dirname "$PRG"`/"$link"
    fi
  done
  PRGDIR=`dirname "$PRG"`
  if [ "$PRGDIR" == "." ]; then
    PRGDIR=`pwd`
  fi
  cd $PRGDIR/..
  GERYON_HOME=`pwd`
  cd $PRGDIR
else
  PRGDIR=$GERYON_HOME/bin
fi

if ! [ -d "$GERYON_HOME/bin" ]; then
  echo "GERYON_HOME $GERYON_HOME is not pointing to the right directory..."
fi

SERVERID=$1

if [ -z "$SERVERID" ]; then
    SERVERID=0
fi

echo "Stopping server $SERVERID using $GERYON_HOME as GERYON_HOME..."

SERVERPROFDIR="$GERYON_HOME/profiles/server$SERVERID"

if [ -f "$SERVERPROFDIR/run/pid" ]; then
    YPID=`cat $SERVERPROFDIR/run/pid`
    kill -1 $YPID
    rm -rf $SERVERPROFDIR/run/pid
    echo "Server $SERVERID stopped."
else
    echo "Server $SERVERID does not seem to be started."
fi
