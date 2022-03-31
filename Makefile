SOURCES= main.c src/scan.c src/ast.c src/parse.c src/cg.c src/cg_x64.c src/cg_llvm.c src/cg_ast.c src/symbol.c src/util/linklist.c src/util/array.c src/util/misc.c

acc: $(SOURCES)
	gcc $(SOURCES) -o acc -g -I include/ -Wall -Wextra -std=c99

clean:
	rm -f acc a.out out.s out.ll

test_x64: acc
	( cd tests; ./test_x64.sh )

test_llvm: acc
	( cd tests; ./test_llvm.sh )

