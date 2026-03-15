#pragma once

/*
 * Kernel call abstraction used by public StratOS user APIs.
 *
 * Today this maps directly to in-process kernel functions.
 * When user/kernel separation is introduced, this macro can
 * be remapped to an SVC/syscall gate without changing API code.
 */

#ifndef STRAT_OS_KERNEL_CALL
    #define STRAT_OS_KERNEL_CALL(fn, ...) fn(__VA_ARGS__)
#endif
