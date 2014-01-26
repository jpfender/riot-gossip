#ifndef __WARN_H
#define __WARN_H

#include <stdio.h>

#if ENABLE_WARN
#define WARN(...) printf(__VA_ARGS__)
#undef ENABLE_WARN
#else
#define WARN(...)
#endif

/** @} */
#endif /* __WARN_H */
