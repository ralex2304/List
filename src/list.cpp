#include "list.h"

#include "list_log/list_log.h"
#include "utils/html.h"
#include "utils/ptr_valid.h"
#include "list_log/list_dot_log.h"

extern ListLogFileData list_log_file;

#define CHECK_AND_RETURN(clause_, error_, ...)  if (clause_) {          \
                                                    res |= error_;      \
                                                    LIST_OK(list, res); \
                                                    __VA_ARGS__;        \
                                                    return res;         \
                                                }

int list_ctor(List* list, size_t init_capacity) {
    assert(list);

    int res = list->OK;

    CHECK_AND_RETURN(list_is_initialised(list), list->ALREADY_INITIALISED);

    init_capacity++; //< fake element

    list->arr = (ListNode*)calloc(init_capacity, sizeof(ListNode));

    CHECK_AND_RETURN(list->arr == nullptr, list->ALLOC_ERR);

    list->capacity = (ssize_t)init_capacity;
    list->arr[0] = {.prev = 0, .elem = ListNode::POISON, .next = 0};

    for (ssize_t i = 1; i < list->capacity; i++) {
        list->arr[i].elem = ListNode::POISON;
        list->arr[i].prev = ListNode::EMPTY_INDEX;
        list->arr[i].next = i + 1;
    }

    list->arr[list->capacity - 1].next = 0;

    list->free_head = 1;

    list->size = 0;
    list->is_linear = true;

    return res | LIST_ASSERT(list);
}

int list_dtor(List* list) {
    int res = LIST_VERIFY(list);
    LIST_OK(list, res);

    fill(list->arr, (size_t)list->capacity, &POISON_LIST_NODE, sizeof(POISON_LIST_NODE));

    FREE(list->arr);

    list->capacity  = list->UNITIALISED_VAL;
    list->free_head = list->UNITIALISED_VAL;
    list->size      = list->UNITIALISED_VAL;
    list->is_linear = false;

    return res;
}

int list_resize(List* list, size_t new_capacity) {
    int res = LIST_ASSERT(list);
    assert((ssize_t)new_capacity != list->capacity);

    res |= list_linearise(list, (ssize_t)new_capacity);

    if (res != list->OK)
        return res;

    return res | LIST_ASSERT(list);
}

int list_linearise(List* list, ssize_t new_capacity) {
    int res = LIST_ASSERT(list);

    if (new_capacity == -1) {
        new_capacity = list->capacity;
    }

    ListNode* new_arr = (ListNode*)calloc((size_t)new_capacity, sizeof(ListNode));

    CHECK_AND_RETURN(new_arr == nullptr, list->ALLOC_ERR);

    new_arr[0] = {.prev = 0, .elem = ListNode::POISON, .next = 0};

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;

    LIST_FOREACH(*list, phys_i, log_i) {
        new_arr[log_i].next = log_i + 1;

        new_arr[log_i + 1] = {.prev = log_i, .elem = list->arr[phys_i].elem, .next = 0};
    }

    LIST_IS_FOREACH_VALID(*list, log_i, {
        res |= list->DAMAGED_PATH;
        LIST_OK(list, res);
        return res;
    });

    new_arr[0].prev = log_i;


    list->free_head = log_i + 1;

    for (phys_i = list->free_head; phys_i < (ssize_t)new_capacity; phys_i++)
        new_arr[phys_i] = {.prev = list->UNITIALISED_VAL, .elem = ListNode::POISON,
                           .next = phys_i + 1};

    new_arr[phys_i - 1].next = 0;

    FREE(list->arr);
    list->arr = new_arr;
    list->capacity = new_capacity;
    list->is_linear = true;

    return res | LIST_ASSERT(list);
}

int list_find_by_logical_index(const List* list, ssize_t logical_i, ssize_t* physical_i) {
    assert(physical_i);
    int res = LIST_ASSERT(list);

    CHECK_AND_RETURN(logical_i >= list->size || logical_i < 0, list->INVALID_POSITION, {
                                                               *physical_i = -1;});
    if (list->is_linear) {
        *physical_i = logical_i + 1;
        return res;
    }

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        if (logical_i-- == 0) {
            *physical_i = phys_i;
            return res;
        }
    }

    LIST_IS_FOREACH_VALID(*list, log_i, {
        res |= list->DAMAGED_PATH;
        LIST_OK(list, res);
    });

    *physical_i = -1;

    return res;
}

int list_find_by_value(const List* list, const Elem_t elem, ssize_t* physical_i) {
    assert(physical_i);
    int res = LIST_ASSERT(list);

    CHECK_AND_RETURN(elem == ListNode::POISON, list->POISON_VAL_FOUND, {
                                               *physical_i = -1;});

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        if (list->arr[phys_i].elem == elem) {
            *physical_i = phys_i;
            return res;
        }
    }

    LIST_IS_FOREACH_VALID(*list, log_i, {
        res |= list->DAMAGED_PATH;
        LIST_OK(list, res);
    });

    *physical_i = -1;

    return res;
}

int list_logical_index_by_physical(const List* list, const ssize_t physical_i, ssize_t* logical_i) {
    assert(logical_i);
    int res = LIST_ASSERT(list);

    CHECK_AND_RETURN(physical_i >= list->capacity || physical_i <= 0, list->INVALID_POSITION, {
                                                                      *logical_i = -1;});
    if (list->is_linear) {
        *logical_i = physical_i - 1;
        return res;
    }

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        if (physical_i == phys_i) {
            *logical_i = log_i;
            return res;
        }
    }

    LIST_IS_FOREACH_VALID(*list, log_i, {
        res |= list->DAMAGED_PATH;
        LIST_OK(list, res);
    });

    *logical_i = -1;

    return res;
}

#define CHECK_ERR_(clause, err) if (clause) res |= err

int list_verify(const List* list) {
    assert(list);

    int res = list->OK;

    CHECK_AND_RETURN(!list_is_initialised(list), list->UNITIALISED);

    bool is_data_valid = true;

    if (!is_ptr_valid(list->arr)) {
        res |= list->DATA_INVALID_PTR;
        is_data_valid = false;
    }

    CHECK_ERR_(list->capacity < list->size + 1, list->LOW_CAPACITY);
    CHECK_ERR_(list->capacity < 0, list->NEGATIVE_CAPACITY);
    CHECK_ERR_(list->size < 0, list->NEGATIVE_SIZE);
    CHECK_ERR_(list->free_head <= 0, list->INVALID_FREE_HEAD);

    if (!is_data_valid)
        return res;

    CHECK_ERR_(list_tail(list) < 0, list->INVALID_TAIL);
    CHECK_ERR_(list_head(list) < 0, list->INVALID_HEAD);

    ssize_t prev_phys_i = 0;

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        CHECK_ERR_(list->arr[phys_i].elem == ListNode::POISON, list->POISON_VAL_FOUND);
        CHECK_ERR_(list->arr[phys_i].prev != prev_phys_i, list->DAMAGED_PATH);

        CHECK_ERR_(list->is_linear && phys_i != log_i + 1, list->INVALID_IS_LINEAR);

        prev_phys_i = phys_i;
    }

    CHECK_ERR_(log_i != list->size, list->DAMAGED_PATH);
    CHECK_ERR_(phys_i < 0, list->DAMAGED_PATH);

    for (phys_i = 1; phys_i < list->capacity; phys_i++) {
        CHECK_ERR_(list->arr[phys_i].prev != -1 && list->arr[phys_i].elem == ListNode::POISON,
                                                                        list->POISON_VAL_FOUND);
        CHECK_ERR_(list->arr[phys_i].prev == -1 && list->arr[phys_i].elem != ListNode::POISON,
                                                                        list->NON_POISON_EMPTY);
    }

    return res;
}
#undef CHECK_ERR_

int list_insert_after(List* list, const size_t position, const Elem_t elem, size_t* inserted_index) {
    int res = LIST_ASSERT(list);

    CHECK_AND_RETURN(list->arr[position].prev == -1, list->INVALID_POSITION);

    res |= list_resize_up(list);
    if (res != list->OK)
        return res;

    *inserted_index = (size_t)list->free_head;
    list->free_head = list->arr[*inserted_index].next;

    list->arr[*inserted_index].prev = (ssize_t)position;
    list->arr[*inserted_index].next = list->arr[position].next;

    list->arr[list->arr[position].next].prev = (ssize_t)*inserted_index;
    list->arr[position].next = (ssize_t)*inserted_index;

    list->arr[*inserted_index].elem = elem;

    list->size++;

    if ((ssize_t)*inserted_index != list_tail(list) && (ssize_t)*inserted_index != list_head(list))
        list->is_linear = false;

    return res | LIST_ASSERT(list);
}

int list_delete(List* list, const size_t position, const bool no_resize) {
    int res = LIST_ASSERT(list);

    CHECK_AND_RETURN(list->arr[position].prev == -1, list->INVALID_POSITION);

    if (!no_resize) {
        res |= list_resize_down(list);
        if (res != list->OK)
            return res;
    }

    list->arr[list->arr[position].prev].next = list->arr[position].next;
    list->arr[list->arr[position].next].prev = list->arr[position].prev;

    list->arr[position].prev = list->UNITIALISED_VAL;
    list->arr[position].next = list->free_head;
    list->free_head = (ssize_t)position;

    list->arr[position].elem = ListNode::POISON;

    list->size--;

    if ((ssize_t)position != list_head(list) && (ssize_t)position != list_tail(list))
        list->is_linear = false;

    return res | LIST_ASSERT(list);
}

#ifndef NDEBUG

int list_ctor_debug(List* list, const VarCodeData var_data, size_t init_capacity) {
    assert(list);

    list->var_data = var_data;

    return list_ctor(list, init_capacity);
}

#endif //< #ifndef NDEBUG


#undef CHECK_AND_RETURN
