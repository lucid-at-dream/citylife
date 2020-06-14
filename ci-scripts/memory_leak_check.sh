#!/bin/bash

tmpfile=$(mktemp)

valgrind --leak-check=full --log-file=$tmpfile $1

leaks=$(grep lost $tmpfile | grep -v -w 0 | wc -l)

if [ $leaks -gt 0 ]
then
  cat $tmpfile
  echo "There are memory leaks."
  exit 1
fi

exit 0
