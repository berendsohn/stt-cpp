#! /bin/sh
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

dir="${0%/*}"
test -n "$dir" && cd "$dir"
./random_points 100 | ./tsp | ./plot_tour.sh
