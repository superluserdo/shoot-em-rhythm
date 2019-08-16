#!/bin/sh
cd `dirname $0`

for i in ../src/modules/*.c; do
  [ -e "$i" ] && echo "$i"
done
