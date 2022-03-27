#! /bin/bash

for input in ./tests/*
do
	./acc $input
done
exit 0
