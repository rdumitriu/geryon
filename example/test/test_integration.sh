#!/bin/sh

CURL=/usr/bin/curl

PRG="$0"

while [ -h "$PRG" ]; do
  ls=`ls -ld "$PRG"`
  link=`expr "$ls" : '.*-> \(.*\)$'`
  if expr "$link" : '/.*' > /dev/null; then
    PRG="$link"
  else
    PRG=`dirname "$PRG"`/"$link"
  fi
done

TESTHOMEDIR=`dirname "$PRG"`
if [ "$TESTHOMEDIR" == "." ]; then
  TESTHOMEDIR=`pwd`
fi

test() {
  testdir=$1
  echo "Running test $1"
  url=`cat $testdir/url`
  expected=`cat $testdir/expected`

  CURL_PARAMS=
  if [ -f $testdir/post-params ]
  then
    prms=`cat $testdir/post-params`
    CURL_PARAMS="-X POST -d \"$prms\""
  fi  

  echo $CURL $CURL_PARAMS "$url"
  v=`$CURL $CURL_PARAMS "$url"`
  
  if [ "$v" != "$expected" ]
  then
    echo "Test $testdir failed ($v NEQ $expected)"
  fi
}



for dir in $TESTHOMEDIR/tdata/*
do
  test $dir
done


