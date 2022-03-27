acc: main.c src/scan.c src/ast.c src/expr.c
	gcc main.c src/scan.c src/ast.c src/expr.c -o acc -g -I include/

clean:
	rm -f acc

test: acc
	tools/autotest.bash
