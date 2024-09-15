#! /bin/sh
# Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
#     and Brown University
# Released under http://opensource.org/licenses/MIT
# May 2014 version

# Reads a traveling salesman tour from stdin and plots it.
exec gnuplot -p -e 'set nokey; set style data linespoints; plot "-"'
