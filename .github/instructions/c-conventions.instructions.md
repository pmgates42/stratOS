---
description: "StratOS C naming, error handling, header, logging, and embedded-printf conventions"
applyTo: "**/*.{c,h}"
---
- Include `generic.h` first; use its types (`uint32_t`, `boolean`, `TRUE`/`FALSE`, `error_type` with `ERR_NO_ERR`/`ERR_INVALID`/...) and helpers (`clr_mem`, `list_cnt`, `BIT`, `GET_BIT`). No libc headers in code built with `EMBEDDED_BUILD`.
- Public headers: `#pragma once`, `extern "C"` guard, and a block comment per function stating parameters, ownership/copy behavior, byte order, and the error returned.
- Public naming: `STRAT_OS_<MODULE>_<verb>` functions (e.g. `STRAT_OS_NET_UDP_decode`), `STRAT_OS_<module>_<noun>_t` types. Subsystems may define their own `*_err_type`.
- Internal naming: lowercase `snake_case`; platform hooks `PLATFORM_<verb>`; app hooks `APP_<verb>`; sim-only code `SIM_<MODULE>_...`.
- Functions return `error_type` (or a module error type), validate pointer params first and return `ERR_INVALID`; outputs go through pointer params.
- Module-private functions and state are `static`; module state uses an `s_` prefix; forward-declare statics at the top of the file.
- Log with `debug_printf("\n[<MODULE>] ...")`. The embedded printf lacks `%ll*` and reliable `%p`; print 64-bit values and pointers as two 32-bit halves.
- Wrap debug-only helpers (declaration and definition) in their feature macro (e.g. `SSCHED_SHOW_DEBUG_DATA`) to avoid `-Wunused-function`.
- Match the indentation and brace style of the file being edited (mostly 4 spaces, Allman braces; `common/uapi/strat_os_net_*` uses tabs).
