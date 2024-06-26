#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "utils/macros.h"

typedef int Elem_t;

#define ELEM_T_PRINTF "%d"

struct ListNode {
    static const Elem_t POISON = __INT_MAX__ - 13;  //< poison value
    static const ssize_t EMPTY_INDEX = -1;          //< value for prev and next

    ssize_t prev = -1;      //< previus element index

    Elem_t elem = POISON;   //< element value

    ssize_t next = -1;      //< next element index
};

// poison node
const ListNode POISON_LIST_NODE = {ListNode::EMPTY_INDEX,
                                   ListNode::POISON,
                                   ListNode::EMPTY_INDEX};

/**
 * @brief Specifies List data
 */
struct List {
    static const ssize_t UNITIALISED_VAL = -1;  //< default value
    static const size_t DEFAULT_CAPACITY = 8;   //< default capacity const

    // error codes
    enum Results {
        OK                   = 0x000000,
        ALLOC_ERR            = 0x000002,

        ALREADY_INITIALISED  = 0x000004,
        UNITIALISED          = 0x000008,
        DATA_INVALID_PTR     = 0x000010,
        POISON_VAL_FOUND     = 0x000020,
        NON_POISON_EMPTY     = 0x000040,
        LOW_CAPACITY         = 0x000200,
        NEGATIVE_CAPACITY    = 0x000400,
        INVALID_CAPACITY     = 0x000800,
        NEGATIVE_SIZE        = 0x001000,
        INVALID_POSITION     = 0x020000,
        DAMAGED_PATH         = 0x040000,
        INVALID_FREE_HEAD    = 0x080000,
        INVALID_TAIL         = 0x100000,
        INVALID_HEAD         = 0x200000,
        INVALID_IS_LINEAR    = 0x400000,
    };

    ssize_t free_head = UNITIALISED_VAL;    //< first free element index

    ListNode* arr = nullptr;    //< data array

    ssize_t capacity = UNITIALISED_VAL;     //< array capacity
    ssize_t size     = UNITIALISED_VAL;     //< number of elements in list

    bool is_linear = false; //< is list linearised (physical index equals sequantional number)

#ifndef NDEBUG
    VarCodeData var_data;   //< keeps data about list variable (name, file, line number)
#endif // #ifndef NDEBUG

};

/**
 * @brief Returns true if list was initialised
 *
 * @param list
 * @return true
 * @return false
 */
inline bool list_is_initialised(const List* list) {
    return !(list->free_head == list->UNITIALISED_VAL &&
             list->capacity  == list->UNITIALISED_VAL &&
             list->size      == list->UNITIALISED_VAL &&
             list->is_linear == false && list->arr == nullptr);
}

/**
 * @brief Returns list head index
 *
 * @param list
 * @return ssize_t
 */
inline ssize_t list_head(const List* list) {
    return list->arr[0].next;
}

/**
 * @brief Returns list tail index
 *
 * @param list
 * @return ssize_t
 */
inline ssize_t list_tail(const List* list) {
    return list->arr[0].prev;
}

/**
 * @brief (Use macros LIST_CTOR or LIST_CTOR_CAP) List constructor
 *
 * @param list
 * @param init_capacity
 * @return int
 */
int list_ctor(List* list, size_t init_capacity = List::DEFAULT_CAPACITY);

/**
 * @brief List destructor
 *
 * @param list
 * @return int
 */
int list_dtor(List* list);

/**
 * @brief Inserts element after physical index
 *
 * @param list
 * @param position physical index
 * @param elem
 * @param inserted_index returns physical index of inserted element
 * @return int
 */
int list_insert_after(List* list, const size_t position, const Elem_t elem, size_t* inserted_index);

/**
 * @brief Inserts element before physical index
 *
 * @param list
 * @param position physical index
 * @param elem
 * @param inserted_index returns physical index of inserted element
 * @return int
 */
inline int list_insert_before(List* list, const size_t position, const Elem_t elem, size_t* inserted_index) {
    return list_insert_after(list, (size_t)list->arr[position].prev, elem, inserted_index);
}

/**
 * @brief Inserts element at the end of the list
 *
 * @param list
 * @param elem
 * @param inserted_index returns physical index of inserted element
 * @return int
 */
inline int list_pushback(List* list, const Elem_t elem, size_t* inserted_index) {
    return list_insert_after(list, (size_t)list_tail(list), elem, inserted_index);
}

/**
 * @brief Inserts element at the beginning of the list
 *
 * @param list
 * @param elem
 * @param inserted_index returns physical index of inserted element
 * @return int
 */
inline int list_pushfront(List* list, const Elem_t elem, size_t* inserted_index) {
    return list_insert_before(list, (size_t)list_head(list), elem, inserted_index);
}

/**
 * @brief Deletes element by physical index
 *
 * @param list
 * @param position physical index
 * @param no_resize will not resize down capacity if true
 * @return int
 */
int list_delete(List* list, const size_t position, const bool no_resize = false);

/**
 * @brief (Use macros LIST_VERIFY) Verifies list data and fields
 *
 * @param list
 * @return int
 */
int list_verify(const List* list);

/**
 * @brief Resizes list (realloc)
 *
 * @param list
 * @param new_capacity
 * @return int
 */
int list_resize(List* list, size_t new_capacity);

/**
 * @brief Linearises array in list. Creates new array and replaces old one
 *
 * @param list
 * @param new_capacity -1 if same as old
 * @return int
 */
int list_linearise(List* list, ssize_t new_capacity = -1);

/**
 * @brief Returns physical index of element with given logical index
 *
 * @param list
 * @param logical_i
 * @param physical_i returnable value. -1 if not found
 * @return int
 */
int list_find_by_logical_index(const List* list, ssize_t logical_i, ssize_t* physical_i);

/**
 * @brief Returns physical index of element with given value (the first one)
 *
 * @param list
 * @param elem
 * @param physical_i returnable value. -1 if not found
 * @return int
 */
int list_find_by_value(const List* list, const Elem_t elem, ssize_t* physical_i);

/**
 * @brief Returns logical index of element with specified logical index
 *
 * @param list
 * @param physical_i
 * @param logical_i returnable value. -1 if not found
 * @return int
 */
int list_logical_index_by_physical(const List* list, const ssize_t physical_i, ssize_t* logical_i);

/**
 * @brief (Use LIST_DUMP macros) Dumps list data to log
 *
 * @param list
 * @param call_data
 */
void list_dump(const List* list, const VarCodeData call_data);

/**
 * @brief Dumps list array to dot file
 *
 * @param list
 * @param img_filename returns image filename
 * @return true
 * @return false
 */
bool list_dump_dot(const List* list, char* img_filename);

/**
 * @brief Prints text error to log by error code
 *
 * @param err_code
 */
void list_print_error(const int err_code);

#ifndef NDEBUG

    /**
     * @brief (Use macros LIST_CTOR or LIST_CTOR_CAP) Constructor wrapper for debug mode
     *
     * @param list
     * @param var_data
     * @param init_capacity
     * @return int
     */
    int list_ctor_debug(List* list, const VarCodeData var_data, size_t init_capacity = List::DEFAULT_CAPACITY);

    /**
     * @brief Constructor
     *
     * @param list
     */
    #define LIST_CTOR(list) list_ctor_debug(list, VAR_CODE_DATA_PTR(list))

    /**
     * @brief Constructor with init capacity
     *
     * @param list
     */
    #define LIST_CTOR_CAP(list, cap) list_ctor_debug(list, VAR_CODE_DATA_PTR(list), cap)

    /**
     * @brief Verifies list data and fields
     *
     * @param list
     */
    #define LIST_VERIFY(list) list_verify(list)

    /**
     * @brief List assert macros (LIST_VERIFY, LIST_OK, and if not ok - return)
     *
     * @param list
     */
    #define LIST_ASSERT(list)       \
                LIST_VERIFY(list);  \
                LIST_OK(list, res); \
                                    \
                if (res != list->OK)\
                    return res

    /**
     * @brief Checks if res is OK, if not, prints error and dump
     *
     * @param list
     * @param res
     */
    #define LIST_OK(list, res)   do {                           \
                                    if (res != list->OK) {      \
                                        list_print_error(res);  \
                                        LIST_DUMP(list);        \
                                    }                           \
                                } while (0)

    /**
     * @brief Prints list dump to log
     *
     * @param list
     */
    #define LIST_DUMP(list) list_dump(list, VAR_CODE_DATA())

#else //< #ifdef NDEBUG

    /**
     * @brief Constructor
     *
     * @param list
     */
    #define LIST_CTOR(list) list_ctor(list)

    /**
     * @brief Constructor with init capacity
     *
     * @param list
     */
    #define LIST_CTOR_CAP(list, cap) list_ctor(list, cap)

    /**
     * @brief Verifies list data and fields (enabled only in DEBUG mode)
     *
     * @param list
     */
    #define LIST_VERIFY(list) List::OK

    /**
     * @brief List assert macros (enabled only in DEBUG mode)
     *
     * @param list
     */
    #define LIST_ASSERT(list) 0

    /**
     * @brief Checks if res is OK (enabled only in DEBUG mode)
     *
     * @param list
     * @param res
     */
    #define LIST_OK(list, res) (void) 0

    /**
     * @brief Prints list dump to log (enabled only in DEBUG mode)
     *
     * @param list
     */
    #define LIST_DUMP(list) (void) 0

#endif //< #ifndef NDEBUG

/**
 * @brief Checks if list capacity is low and resizes it
 *
 * @param list
 * @return int
 */
inline int list_resize_up(List* list) {
    int res = LIST_ASSERT(list);

    ssize_t new_capacity = list->capacity;

    while (list->size >= new_capacity - 1 - 1)
        new_capacity = (new_capacity - 1) * 2 + 1;

    if (new_capacity != list->capacity)
        return res | list_resize(list, (size_t)new_capacity);

    return res;
}

/**
 * @brief Checks if list capacity is too big and resizes it
 *
 * @param list
 * @return int
 */
inline int list_resize_down(List* list) {
    int res = LIST_ASSERT(list);

    ssize_t new_capacity = list->capacity;

    while (list->size < (new_capacity - 1) / 2)
        new_capacity = (new_capacity - 1) / 2 + 1;

    if (new_capacity != list->capacity)
        return res | list_resize(list, (size_t)new_capacity);

    return res;
}

#define LIST_FOREACH(list_, phys_i_, log_i_)                            \
    for (; phys_i_ > 0 && log_i_ <= (list_).size; phys_i_ = (list_).arr[phys_i_].next, (log_i_)++)

#define LIST_IS_FOREACH_VALID(list_, log_i_, ...)   do {    \
            if (log_i_ != (list_).size) {                   \
                __VA_ARGS__;                                \
            }                                               \
        } while (0)

#endif //< #ifndef LIST_H_
