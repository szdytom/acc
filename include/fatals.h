#ifndef ACC_FATALS_H
#define ACC_FATALS_H

#include <stddef.h>
#include <stdnoreturn.h>

noreturn void fail_unreachable(const char *func_name);
noreturn void fail_type(int line);
noreturn void fail_todo(const char *func_name);
noreturn void fail_target(const char *target_name);
noreturn void fail_malloc(const char *func_name);
noreturn void fail_ir_op(int op, const char *func_name);
noreturn void fail_ast_op(int op, const char *func_name);
noreturn void fail_ce_expect(int line, const char *expected, const char *got);
noreturn void fail_ce(int line, const char *reason);
noreturn void fail_char(int line, int c);

#endif
