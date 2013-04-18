#!/bin/sh
# Generate configuration script and launch it

autoconf configure.ac > configure
chmod +x configure
./configure