scanner: main.c src/scan.c
	gcc main.c src/scan.c -o scanner -g -I include/

clean:
	rm -f scanner
