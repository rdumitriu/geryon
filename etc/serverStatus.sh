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

SERVERNAMES=$1

if [ -z "$SERVERNAMES" ]; then
    for f in `ls $GERYON_HOME/profiles`
    do
        if [ "$f" != "template" ]; then
          SERVERNAMES="$SERVERNAMES $f"
        fi
    done;
fi

for f in $SERVERNAMES
do
    if [ -f "$GERYON_HOME/profiles/$f/run/pid" ]; then
        YPID=`cat $GERYON_HOME/profiles/$f/run/pid`
        c=`ps -adef | grep geryon | grep $YPID | wc -l`
        if [ $c -eq 1 ]; then
            echo "Geryon $f seems to be running"
        else
            echo "Geryon $f seems stopped (but pid file exists)."
        fi
    else
        echo "Geryon $f seems stopped."
    fi
done;

