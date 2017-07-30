#ifdef _DEBUG

#define ASSERT(expr) ((void)((expr) ? 0 : __my_assert(#expr, __FILE__, __LINE__)))

#define HYP_DBG(x) hyp_debug x

#else

#define ASSERT(expr)
#define HYP_DBG(x)

#endif

int __my_assert(const char *expr, const char *file, int line);

void hyp_debug(const char *str, ...) __attribute__((format(printf, 1, 2)));

#if defined(__TOS__) || defined(__atarist__)
#include <mint/arch/nf_ops.h>
#else
#define nf_debugprintf(fmt, ...) (-1)
#define nf_debugvprintf(fmt, args) (-1)
#endif

#define unreachable() abort()
