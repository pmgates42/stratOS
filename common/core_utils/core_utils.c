#include "generic.h"
#include "uapi/strat_os_core_utils.h"

error_type STRAT_OS_CORE_linked_list_init(STRAT_OS_core_linked_list_node_t *list)
{
    if (list == NULL)
        return ERR_INVALID;

    list->next = NULL;
    list->previous = NULL;

    return ERR_NO_ERR;
}

error_type STRAT_OS_CORE_linked_list_append(
    STRAT_OS_core_linked_list_node_t *list,
    STRAT_OS_core_linked_list_node_t *node)
{
    STRAT_OS_core_linked_list_node_t *tail;

    if (list == NULL || node == NULL)
        return ERR_INVALID;

    for (tail = list; tail->next != NULL; tail = tail->next)
        ;

    node->next = NULL;
    node->previous = tail;
    tail->next = node;

    return ERR_NO_ERR;
}

error_type STRAT_OS_CORE_linked_list_remove(STRAT_OS_core_linked_list_node_t *node)
{
    STRAT_OS_core_linked_list_node_t *prev_node_ptr;
    STRAT_OS_core_linked_list_node_t *next_node_ptr;

    if (node == NULL)
        return ERR_INVALID;

    prev_node_ptr = node->previous;
    next_node_ptr = node->next;

    if (prev_node_ptr != NULL)
        prev_node_ptr->next = next_node_ptr;

    if (next_node_ptr != NULL)
        next_node_ptr->previous = prev_node_ptr;
    
    node->previous = NULL;
    node->next = NULL;

    return ERR_NO_ERR;
}

void STRAT_OS_CORE_linked_list_destroy(STRAT_OS_core_linked_list_node_t *list)
{
    STRAT_OS_core_linked_list_node_t *node;
    STRAT_OS_core_linked_list_node_t *next_node_ptr;

    if (list == NULL)
        return;

    for (node = list->next; node != NULL; node = next_node_ptr)
    {
        next_node_ptr = node->next;
        node->previous = NULL;
        node->next = NULL;
    }

    list->next = NULL;
    list->previous = NULL;
}
