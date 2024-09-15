#! /bin/sh
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

set -e
dir="${0%/*}"/..
test -n "$dir" && cd "$dir"
mkdir -p standalone
script='
s|DTREE_SELFADJUST_|DTREE_|
s|^#include "dtree\(/selfadjust\)\{0,1\}/\([-.a-z]*\)"$|#include "\2"|
/^#include "util\/disallow_copy_and_assign.h"$/d
/^#include "util\/scoped.h"$/d
s|namespace selfadjust|namespace dtree|
s|dtree::||g
s|util::||g
s|^\( *\)DISALLOW_COPY_AND_ASSIGN(\([A-Za-z]*\));$|\1\2(const \2\&);\
\1void operator=(const \2\&);|
'
for h in dtree/type.h dtree/selfadjust/seq.h dtree/selfadjust/seq-inl.h dtree/selfadjust/tree.h dtree/selfadjust/tree-inl.h dtree/selfadjust/tree-extra.h util/scoped.h
do
	sed -e "$script" $h >standalone/${h##*/}
done
sed -e '
1,/^namespace util {$/d
/^}  \/\/ namespace util$/,$d
' standalone/scoped.h >standalone/a
sed -e "$script" -e '/^namespace dtree {$/r standalone/a' standalone/tree-extra.h >standalone/b
mv -f standalone/b standalone/tree-extra.h
rm -f standalone/a standalone/scoped.h
