#ifndef ACC_FATALS_H
#define ACC_FATALS_H

#include <stddef.h>
#include <stdnoreturn.h>

void* malloc_or_fail(size_t s, const char *func_name);
noreturn void fail_target(const char *target_name);
noreturn void fail_malloc(const char *func_name);
noreturn void fail_ast_op(int op, const char *func_name);
noreturn void fail_ce_expect(const char *expected, const char *got);
noreturn void fail_ce(const char *reason);
noreturn void fail_char(int c);

#endif
