#!/bin/bash

tmpfile=$(mktemp)

valgrind --leak-check=full $1 2> $tmpfile

cat $tmpfile

leaks=$(echo $(grep lost $tmpfile | cut -d' ' -f2- | grep -ow "[0-9]*") | sed 's/\n//' | sed 's/ //g')

if [ $leaks -gt 0 ]
then
  echo "There are memory leaks."
  exit -1
fi

exit 0

