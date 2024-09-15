#! /bin/sh
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

set -e
dir="${0%/*}"
test -n "$dir" && cd "$dir"
(
set +e
for c in vf vc
do
	i=0
	while [ $i -lt 60 ]
	do
		../../bin/flow $c case/$i.txt
		i=$((i + 1))
	done
done
) 2>&1 | diff -c expected.txt -
i=0
while [ $i -lt 100 ]
do
	# printf '%2d%%\n' $i
	../../bin/random_network 100 1000 10000 $i | ../../bin/flow pr | ../../bin/flow vp
	../../bin/random_network 100 1000 10000 $i | ../../bin/flow sr | ../../bin/flow vp
	i=$((i + 1))
done
printf 'OK %s\n' "$0"
