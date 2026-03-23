#include "cpp_runtime.h"

typedef void (*ctor_t)(void);

extern ctor_t __init_array_start[];
extern ctor_t __init_array_end[];

void cpp_init(void)
{
       for (ctor_t *ctor = __init_array_start; ctor < __init_array_end; ctor++)
              (*ctor)();
}
