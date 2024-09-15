#! /bin/sh
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

set -e
dir="${0%/*}"/..
test -n "$dir" && cd "$dir"
find example \( -name '*.cc' -or -name '*.cpp' -or -name '*.h' \) -exec sed -i~ 's/__attribute__(([^()]*\(([^()]*)\)*))//g' {} +
