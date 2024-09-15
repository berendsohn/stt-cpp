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
../../bin/tsp <<EOF
0 1
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
1 0
EOF
printf '%s\n' ---
../../bin/tsp </
printf '%s\n' ---
../../bin/tsp <<EOF
a
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
0 a
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
0
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
0 0
EOF
printf '%s\n' ---
../../bin/tsp <<EOF
0 0
0 0.5
0.5 0
0.5 0.5
EOF
) 2>&1 | diff -c expected.txt -
printf 'OK %s\n' "$0"
