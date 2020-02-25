#!/bin/sh

set -x;

DESTDIR=$HOME/dba_dist
COMMON_NAME=common-1.2.0
DBA_NAME=dba-1.2.0

WEBDIR=/home/services/httpd/html/dba
cp -a $WEBDIR $TMP/dba_tmp;

#copy new source tarballs and zip files to download section
mkdir $WEBDIR/download
cp $DESTDIR/$COMMON_NAME.* $WEBDIR/download;
cp $DESTDIR/$DBA_NAME.* $WEBDIR/download;

#delete website and create new one
cd $WEBDIR && rm -rf *;
tar zxvf $DESTDIR/web-$DBA_NAME.tar.gz;
mv $TMP/dba_tmp/download ./

#set file permissions
chmod 755 `find . -type d`
chmod 644 `find . -type f`
