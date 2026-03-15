#pragma once

#include "generic.h"

/*
 * Application hook called by kernel init to let the application
 * configure scheduler tasks and other app-owned startup behavior.
 *
 * Applications should implement this symbol. A weak default
 * implementation is provided by the OS.
 */
boolean APP_sched_configure(void);
