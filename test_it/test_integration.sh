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

if [ "$TESTHOMEDIR" = "." ]
then
  TESTHOMEDIR=`pwd`
fi


test() {
  testdir=$1
  echo "Running test $1"
  url=`cat $testdir/url`
  expected=`cat $testdir/expected`

  if [ -f $testdir/post-params ]
  then
    prms=`cat $testdir/post-params`
    v=`$CURL -s $hdrs -X POST -d "$prms" "$url"`
  else
    if [ -f $testdir/json ]
    then
      v=`$CURL -s -H 'Accept: application/json' -H 'Content-type: application/json' -H 'Transfer-Encoding: chunked' -H 'Expect: 100-continue' -X POST --data @$testdir/json "$url"`
    else
      if [ -f $testdir/put-json ]
      then
        v=`$CURL -s -H 'Accept: application/json' -H 'Content-type: application/json' -H 'Transfer-Encoding: chunked' -H 'Expect: 100-continue' -X PUT --data @$testdir/put-json "$url"`
      else
        v=`$CURL -s "$url"`
      fi
    fi
  fi  
  
  if [ "$v" != "$expected" ]
  then
    echo "Test $testdir failed ($v NEQ $expected)"
  fi
}


for dir in $TESTHOMEDIR/tdata/*
do
  test $dir
done

