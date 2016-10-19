#! /bin/sh
#
# $Id: autogen.sh,v 1.16 2009-11-15 20:00:56 al-guest Exp $

set -x

aclocal
[ "$?" != 0 ] && echo "aclocal-$AUTOMAKEVER not available or failed!" && exit 1
autoheader || exit 1
automake --add-missing --force --gnu || exit 1
autoconf || exit 1
