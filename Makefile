SOURCES= main.c src/scan.c src/ast.c src/parse.c src/cg.c src/cg_x64.c src/cg_llvm.c src/cg_ast.c src/symbol.c src/util/linklist.c src/util/array.c src/util/misc.c

acc: $(SOURCES)
	gcc $(SOURCES) -o acc -g -I include/

clean:
	rm -f acc a.out out.s out.ll

test_x64: acc
	tools/test_x64.bash

test_llvm: acc
	tools/test_llvm.bash

