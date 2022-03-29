#! /bin/bash

set -e
for input in ./tests/*
do
	./acc x86_64 $input
	gcc out.s
	./a.out
done
exit 0
