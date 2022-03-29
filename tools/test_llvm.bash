#! /bin/bash

set -e
for input in ./tests/*
do
	./acc llvm $input
	clang out.ll -w
	./a.out
done
exit 0
