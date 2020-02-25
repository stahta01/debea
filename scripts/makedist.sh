#!/bin/sh

set -x;

SVNREP=svn+ssh://svn@vortex.candela.com.pl/repos/production/sources
SVNBRANCH=/branches/devel/dba_1_2
DESTDIR=$HOME/dba_dist
COMMON_NAME=common-1.2.0
DBA_NAME=dba-1.2.0
ACLOCAL_FLAGS="-I $HOME/share/aclocal"

COMMON_DIR=$DESTDIR/$COMMON_NAME
DBA_DIR=$DESTDIR/$DBA_NAME

DBA_SUPPORT_DIR=$DBA_DIR/support

BAKEFILE_PATHS=$DBA_DIR/support:$DESTDIR/common_install/share/bakefile
export BAKEFILE_PATHS;

#if [ 0 == 1 ]; then
rm -rf $DESTDIR;

# get source trees
svn export $SVNREP/$SVNBRANCH/common $COMMON_DIR
svn export $SVNREP/$SVNBRANCH/dba $DBA_DIR

# add bakefile support files
svn export $SVNREP/trunk/bkltemplates $DBA_SUPPORT_DIR

# create project files
cd $COMMON_DIR;
bakefilize --copy
bakefile -f msvc common.bkl
bakefile -f msvc6prj common.bkl
bakefile -f autoconf common.bkl
aclocal -I $DBA_SUPPORT_DIR/aclocal $ACLOCAL_FLAGS
autoconf

# we have to compile common to run dba configure
mkdir $DESTDIR/common_build
cd $DESTDIR/common_build;
$COMMON_DIR/configure --prefix=$DESTDIR/common_install;
make;
make install;

# create proejct files for dba
cd $DBA_DIR;
bakefilize --copy
bakefile_gen -d dba.bkgen
aclocal -I $DBA_SUPPORT_DIR/aclocal $ACLOCAL_FLAGS
autoconf


# add cr/lf to windows project files
unix2dos `find $DESTDIR -name "*.vc" -o -name "*.dsp" -o -name "*.dsw"`

# create documentation for distribution tarballs
DOCS_DIR=$TMP/dba_docs
rm -rf $DOCS_DIR;
mkdir -p $DOCS_DIR/{build,dist,web};
cd $DOCS_DIR/build;
$DBA_DIR/configure --enable-docs --with-doc_section=doc --prefix=$DOCS_DIR/dist --with-common-config=$DESTDIR/common_install/bin/common-config
make doc;
make doc-install;

# generate new website
mv $DOCS_DIR/dist/share/doc/dba/html $DBA_DIR/docs/html
cd $DOCS_DIR;
rm -rf $DOCS_DIR/build; mkdir $DOCS_DIR/build;
cd $DOCS_DIR/build;
$DBA_DIR/configure --enable-docs --with-doc_section=web --prefix=$DOCS_DIR/web --with-common-config=$DESTDIR/common_install/bin/common-config
make doc;
make doc-install;

# create tarball with new version of website
cd $DOCS_DIR/web/share/doc/dba/
tar zcvf web-$DBA_NAME.tar.gz -C html .;
mv web-$DBA_NAME.tar.gz $DESTDIR;

# create source archives
cd $DESTDIR;
tar zcvf $COMMON_NAME.tar.gz $COMMON_NAME;
tar zcvf $DBA_NAME.tar.gz $DBA_NAME;

# stupid zip cannot create files with dots in name
mv $COMMON_NAME common;
mv $DBA_NAME dba;
zip -r common common;
zip -r dba dba;
mv common.zip $COMMON_NAME.zip;
mv dba.zip $DBA_NAME.zip;


