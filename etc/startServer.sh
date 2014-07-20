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

echo "Starting server with ID $SERVERID using $GERYON_HOME as GERYON_HOME..."

SERVERPROFDIR="$GERYON_HOME/profiles/server$SERVERID"

if ! [ -d $SERVERPROFDIR ]; then
    echo "Server $SERVERID is NOT configured. Create the profile first."
    exit 1;
fi

if [ -f "$SERVERPROFDIR/run/pid" ]; then
    echo "Pid file [$SERVERPROFDIR/run/pid] exists, check to see if this is not started already"
    YPID=`cat $SERVERPROFDIR/run/pid`
    c=`ps -adef | grep geryon | grep $YPID | wc -l`
    if [ $c -eq 1 ]; then
        echo "Instance $SERVERID seems to be running. Stop it first."
        exit 1;
    else
        echo "Found stalled pid file, removing ..."
        rm -rf $SERVERPROFDIR/run/pid
    fi
fi

if ! [ -d "$SERVERPROFDIR/run" ]; then
    mkdir $SERVERPROFDIR/run
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GERYON_HOME/lib
$PRGDIR/geryon -h $GERYON_HOME -i $SERVERID &
YPID=$!
echo "$YPID" > $SERVERPROFDIR/run/pid

c=0
while [ $c -eq 0 ]
do
    sleep 1
    c=`ps -adef | grep geryon | grep $YPID | wc -l`
done;

echo "Server $SERVERID started."
