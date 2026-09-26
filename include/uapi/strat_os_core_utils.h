#pragma once

#include "generic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STRAT_OS_core_linked_list_node
{
    struct STRAT_OS_core_linked_list_node *next;
    struct STRAT_OS_core_linked_list_node *previous;
} STRAT_OS_core_linked_list_node_t;

void STRAT_OS_CORE_linked_list_init(STRAT_OS_core_linked_list_node_t *list);
void STRAT_OS_CORE_linked_list_append(
    STRAT_OS_core_linked_list_node_t *list,
    STRAT_OS_core_linked_list_node_t *node);
void STRAT_OS_CORE_linked_list_remove(STRAT_OS_core_linked_list_node_t *node);
void STRAT_OS_CORE_linked_list_destroy(STRAT_OS_core_linked_list_node_t *list);

#ifdef __cplusplus
}
#endif