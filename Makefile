acc: main.c src/scan.c src/ast.c src/expr.c src/cg.c src/util/linklist.c
	gcc main.c src/scan.c src/ast.c src/expr.c src/cg.c src/util/linklist.c -o acc -g -I include/

clean:
	rm -f acc

test: acc
	tools/autotest.bash
