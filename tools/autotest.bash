#! /bin/bash

set -e
for input in ./tests/*
do
	./acc $input
	gcc out.s -o a
	./a
done
exit 0
