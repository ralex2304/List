#include "list.h"

extern LogFileData log_file;

int list_ctor(List* list, size_t init_capacity) {
    assert(list);

    int res = list->OK;

    if (list_is_initialised(list)) {
        return res | list->ALREADY_INITIALISED;
    }

    init_capacity++; //< fake element

    list->arr = (ListNode*)calloc(init_capacity, sizeof(ListNode));

    if (list->arr == nullptr) {
        res |= list->ALLOC_ERR;
        LIST_OK(list, res);
        return res;
    }

    list->capacity = init_capacity;
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

    fill(list->arr, list->capacity, &POISON_LIST_NODE, sizeof(POISON_LIST_NODE));

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

    ListNode* new_arr = nullptr;
    ssize_t new_free_head = -1;
    res |= list_linearise(list, &new_arr, &new_free_head, new_capacity);

    if (res != list->OK)
        return res;

    if (new_arr == nullptr) {
        res |= list->ALLOC_ERR;
        LIST_OK(list, res);
        return res;
    }

    FREE(list->arr);

    list->arr = new_arr;
    list->capacity = new_capacity;
    list->free_head = new_free_head;
    list->is_linear = true;

    return res | LIST_ASSERT(list);
}

int list_linearise(List* list, ListNode** dest, ssize_t* dest_free_head, size_t dest_capacity) {
    int res = LIST_ASSERT(list);
    assert(dest);
    assert(dest_free_head);
    assert(*dest == nullptr);
    assert((ssize_t)dest_capacity > list->size + 1);

    *dest = (ListNode*)calloc(dest_capacity, sizeof(ListNode));

    if (*dest == nullptr) {
        res |= list->ALLOC_ERR;
        LIST_OK(list, res);
        return res;
    }

    (*dest)[0] = {.prev = 0, .elem = ListNode::POISON, .next = 0};

    ssize_t old_i = list_head(list);
    ssize_t new_i = 0;

    while (old_i > 0) {
        (*dest)[new_i].next = new_i + 1;

        (*dest)[++new_i] = {.prev = new_i - 1, .elem = list->arr[old_i].elem, .next = 0};

        if (new_i > list->size) {
            res |= list->DAMAGED_PATH;
            LIST_OK(list, res);
            return res;
        }
        old_i = list->arr[old_i].next;
    }

    (*dest)[0].prev = new_i;

    *dest_free_head = new_i + 1;

    for (new_i = *dest_free_head; new_i < (ssize_t)dest_capacity; new_i++)
        (*dest)[new_i] = {.prev = list->UNITIALISED_VAL, .elem = ListNode::POISON, .next = new_i + 1};

    (*dest)[new_i - 1].next = 0;

    return res | LIST_ASSERT(list);;
}

int list_find_by_logical_index(const List* list, ssize_t logical_i, ssize_t* physical_i) {
    assert(physical_i);
    int res = LIST_ASSERT(list);

    if (logical_i >= list->size || logical_i < 0) {
        res |= list->INVALID_POSITION;
        LIST_OK(list, res);
        *physical_i = -1;
        return res;
    }

    ssize_t i = list_head(list);
    ssize_t log_i = 0;

    while (i > 0) {
        if (logical_i-- == 0) {
            *physical_i = i;
            return res;
        }

        if (++log_i > list->size) {
            res |= list->DAMAGED_PATH;
            LIST_OK(list, res);
            *physical_i = -1;
            return res;
        }

        i = list->arr[i].next;
    }

    *physical_i = -1;

    return res;
}

int list_find_by_value(const List* list, const Elem_t elem, ssize_t* physical_i) {
    assert(physical_i);
    int res = LIST_ASSERT(list);

    if (elem == ListNode::POISON) {
        res |= list->POISON_VAL_FOUND;
        LIST_OK(list, res);
        *physical_i = -1;
        return res;
    }

    ssize_t i = list_head(list);
    ssize_t log_i = 0;

    while (i > 0) {
        if (list->arr[i].elem == elem) {
            *physical_i = i;
            return res;
        }

        if (++log_i > list->size) {
            res |= list->DAMAGED_PATH;
            LIST_OK(list, res);
            *physical_i = -1;
            return res;
        }

        i = list->arr[i].next;
    }

    *physical_i = -1;

    return res;
}

int list_logical_index_by_physical(const List* list, const ssize_t physical_i, ssize_t* logical_i) {
    assert(physical_i);
    int res = LIST_ASSERT(list);

    if (physical_i >= list->capacity || physical_i <= 0) {
        res |= list->INVALID_POSITION;
        LIST_OK(list, res);
        *logical_i = -1;
        return res;
    }

    ssize_t i = list_head(list);
    ssize_t log_i = 0;

    while (i > 0) {
        if (physical_i == i) {
            *logical_i = log_i;
            return res;
        }

        if (++log_i > list->size) {
            res |= list->DAMAGED_PATH;
            LIST_OK(list, res);
            *logical_i = -1;
            return res;
        }

        i = list->arr[i].next;
    }

    *logical_i = -1;

    return res;
}

#define LOG_(...) log_printf(&log_file, __VA_ARGS__)

void list_dump(const List* list, const VarCodeData call_data) {
    assert(list);

    LOG_(HTML_BEGIN);

    LOG_("    list_dump() called from %s:%d %s\n"
         "    %s[%p] initialised in %s:%d %s \n",
         call_data.file, call_data.line, call_data.func,
         list->var_data.name, list,
         list->var_data.file, list->var_data.line, list->var_data.func);

    LOG_("    {\n");
    LOG_("    real capacity  = %zd\n", list->capacity);
    LOG_("    size           = %zd\n", list->size);
    LOG_("    head           = %zd\n", list_head(list));
    LOG_("    tail           = %zd\n", list_tail(list));
    LOG_("    free_head      = %zd\n", list->free_head);
    LOG_("    is_linear      = %s\n",  list->is_linear ? "true" : "false");
    LOG_("        {\n");

    if (!is_ptr_valid(list->arr)) {

        if (list_is_initialised(list))
            LOG_(HTML_RED("        can't read (invalid pointer)\n"));

        LOG_("        }\n"
             "    }\n" HTML_END);
        return;
    }

    LOG_("        ""  i | prev | next | elem\n");

    for (ssize_t i = 0; i < list->capacity; i++) {
        LOG_("        ""%3zd | %4zd | %4zd | " ELEM_T_PRINTF "\n",
             i, list->arr[i].prev, list->arr[i].next, list->arr[i].elem);
    }

    LOG_("        }\n");

    LOG_("    Elements:");

    ssize_t i = list_head(list);
    ssize_t log_i = 0;

    while (i != 0) {
        LOG_(" " ELEM_T_PRINTF, list->arr[i].elem);

        if (++log_i > list->size) {
            LIST_OK(list, list->DAMAGED_PATH);
            return;
        }
        i = list->arr[i].next;
    }

    LOG_("\n"
         "    }\n" HTML_END);
}
#undef LOG_

#define CHECK_ERR_(clause, err) if (clause) res |= err

int list_verify(const List* list) {
    assert(list);

    int res = list->OK;

    if (!list_is_initialised(list)) {
        res |= list->UNITIALISED;
        return res;
    }

    bool is_data_valid = true;

    if (!is_ptr_valid(list->arr)) {
        res |= list->DATA_INVALID_PTR;
        is_data_valid = false;
    }

    CHECK_ERR_(list->capacity < list->size + 1, list->LOW_CAPACITY);
    CHECK_ERR_(list->capacity < 0, list->NEGATIVE_CAPACITY);
    CHECK_ERR_(list->size < 0, list->NEGATIVE_SIZE);
    CHECK_ERR_(list->free_head <= 0, list->INVALID_FREE_HEAD);

    if (is_data_valid) {
        CHECK_ERR_(list_tail(list) < 0, list->INVALID_TAIL);
        CHECK_ERR_(list_head(list) < 0, list->INVALID_HEAD);

        ssize_t i = list_head(list);
        ssize_t bef_i = 0;
        ssize_t log_i = 0;

        while (i > 0) {
            CHECK_ERR_(list->arr[i].elem == ListNode::POISON, list->POISON_VAL_FOUND);
            CHECK_ERR_(list->arr[i].prev != bef_i, list->DAMAGED_PATH);

            CHECK_ERR_(list->is_linear && i != log_i + 1, list->INVALID_IS_LINEAR);

            if (++log_i > list->size) {
                res |= list->DAMAGED_PATH;
                break;
            }
            bef_i = i;
            i = list->arr[i].next;
        }

        CHECK_ERR_(log_i != list->size, list->DAMAGED_PATH);
        CHECK_ERR_(i < 0, list->DAMAGED_PATH);

        for (i = 1; i < list->capacity; i++) {
            CHECK_ERR_(list->arr[i].prev != -1 && list->arr[i].elem == ListNode::POISON,
                                                                           list->POISON_VAL_FOUND);
            CHECK_ERR_(list->arr[i].prev == -1 && list->arr[i].elem != ListNode::POISON,
                                                                           list->NON_POISON_EMPTY);
        }
    }

    return res;
}

#undef CHECK_ERR_

#define PRINT_ERR_(code, descr)  if ((err_code) & List::code)                                    \
                                    log_printf(&log_file,                                         \
                                               HTML_TEXT(HTML_RED("!!! " #code ": " descr "\n")));
void list_print_error(const int err_code) {
    if (err_code == List::OK) {
        log_printf(&log_file, HTML_TEXT(HTML_GREEN("No error\n")));
    } else {
        PRINT_ERR_(ALREADY_INITIALISED,  "Constructor called for already initialised or corrupted list");
        PRINT_ERR_(UNITIALISED,          "List is not initialised");
        PRINT_ERR_(DATA_INVALID_PTR,     "list.arr pointer is not valid for writing");
        PRINT_ERR_(ALLOC_ERR,            "Can't allocate memory");
        PRINT_ERR_(POISON_VAL_FOUND,     "There is poison value in list");
        PRINT_ERR_(NON_POISON_EMPTY,     "Empty element is not poison");
        PRINT_ERR_(LOW_CAPACITY,         "list.size > list.capacity - 1");
        PRINT_ERR_(NEGATIVE_CAPACITY,    "Negative list.capacity");
        PRINT_ERR_(INVALID_CAPACITY,     "Invalid capacity given");
        PRINT_ERR_(NEGATIVE_SIZE,        "Negative list.size");
        PRINT_ERR_(INVALID_POSITION,     "Invalid physical index given");
        PRINT_ERR_(DAMAGED_PATH,         "List is damaged. Invalid path");
        PRINT_ERR_(INVALID_FREE_HEAD,    "Invalid free_head field");
        PRINT_ERR_(INVALID_TAIL,         "Invalid tail field");
        PRINT_ERR_(INVALID_HEAD,         "Invalid head field");
        PRINT_ERR_(INVALID_IS_LINEAR,    "is_linear flag is true, but list isn't linear");
    }
}
#undef PRINT_ERR_

int list_insert_after(List* list, const size_t position, const Elem_t elem, size_t* inserted_index) {
    int res = LIST_ASSERT(list);

    if (list->arr[position].prev == -1) {
        res |= list->INVALID_POSITION;
        return res;
    }

    res |= list_resize_up(list);
    if (res != list->OK)
        return res;

    *inserted_index = list->free_head;
    list->free_head = list->arr[*inserted_index].next;

    list->arr[*inserted_index].prev = position;
    list->arr[*inserted_index].next = list->arr[position].next;

    list->arr[list->arr[position].next].prev = *inserted_index;
    list->arr[position].next = *inserted_index;

    list->arr[*inserted_index].elem = elem;

    list->size++;

    if ((ssize_t)*inserted_index != list_tail(list) && (ssize_t)*inserted_index != list_head(list))
        list->is_linear = false;

    return res | LIST_ASSERT(list);
}

int list_delete(List* list, const size_t position, const bool no_resize) {
    int res = LIST_ASSERT(list);

    if (list->arr[position].prev == -1) {
        res |= list->INVALID_POSITION;
        return res;
    }

    if (!no_resize) {
        res |= list_resize_down(list);
        if (res != list->OK)
            return res;
    }

    list->arr[list->arr[position].prev].next = list->arr[position].next;
    list->arr[list->arr[position].next].prev = list->arr[position].prev;

    list->arr[position].prev = list->UNITIALISED_VAL;
    list->arr[position].next = list->free_head;
    list->free_head = position;

    list->arr[position].elem = ListNode::POISON;

    list->size--;

    if ((ssize_t)position != list_head(list) && (ssize_t)position != list_tail(list))
        list->is_linear = false;

    return res | LIST_ASSERT(list);
}

#ifdef DEBUG

int list_ctor_debug(List* list, const VarCodeData var_data, size_t init_capacity) {
    assert(list);

    list->var_data = var_data;

    return list_ctor(list, init_capacity);
}

#endif //< #ifdef DEBUG
