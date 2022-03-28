SOURCES= main.c src/scan.c src/ast.c src/parse.c src/cg.c src/symbol.c src/util/linklist.c src/util/array.c

acc: $(SOURCES)
	gcc $(SOURCES) -o acc -g -I include/

clean:
	rm -f acc

test: acc
	tools/autotest.bash
