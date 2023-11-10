#include "list.h"

extern LogFileData log_file;

#ifdef DEBUG

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

    LOG_("    Ordered elements:");

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        LOG_(" " ELEM_T_PRINTF, list->arr[phys_i].elem);
    }
    LOG_("\n    Physical indexes:");

    phys_i = list_head(list);
    log_i = 0;
    LIST_FOREACH(*list, phys_i, log_i) {
        LOG_(" %zu", phys_i);
    }

    LOG_("\n"
         "    }\n" HTML_END);

    char img_filename[log_file.MAX_FILENAME_LEN] = {};

    list_dump_dot(list, img_filename);

    LOG_("<img src=\"../../%s\">\n", img_filename);
}
#undef LOG_

#define FPRINTF_(...) if (fprintf(file, __VA_ARGS__) == 0) return false
bool list_dump_dot(const List* list, char* img_filename) {
    #define BACKGROUND_COLOR "\"#1f1f1f\""
    #define FONT_COLOR       "\"#000000\""
    #define NODE_PREFIX      "elem_"
    #define NODE_PARAMS      "shape=\"plaintext\", style=\"filled\", fillcolor=\"#6e7681\""
    #define ZERO_NODE_PARAMS "shape=\"plaintext\", style=\"filled\", fillcolor=\"#6e7681\", color=yellow"

    assert(list);

    static size_t dot_number = 0;

    char dot_filename[log_file.MAX_FILENAME_LEN] = {};

    size_t str_len = strncat_len(dot_filename, log_file.timestamp_dir, log_file.MAX_FILENAME_LEN);
    snprintf(dot_filename + str_len, log_file.MAX_FILENAME_LEN - str_len,
             "%zd", dot_number);
    str_len = strncat_len(dot_filename, ".dot", log_file.MAX_FILENAME_LEN);

    FILE* file = fopen(dot_filename, "wb");
    if (file == nullptr)
        return false;

    FPRINTF_("digraph List{\n"
             "    graph [bgcolor=" BACKGROUND_COLOR ", splines=ortho];\n"
             "    node[color=white, fontcolor=" FONT_COLOR ", fontsize=14, fontname=\"verdana\"];\n\n");

    FPRINTF_(NODE_PREFIX "0 [" ZERO_NODE_PARAMS ", label=< <table cellspacing=\"0\">\n"
                                                            "<tr><td>head = %zd </td></tr>\n"
                                                            "<tr><td>tail = %zd </td></tr>\n"
                                                            "<tr><td>free_tail = %zd</td></tr>\n"
                                                            "</table>>];\n\n",
             list_head(list), list_tail(list), list->free_head);

    for (ssize_t phys_i = 1; phys_i < list->capacity; phys_i++) {
        FPRINTF_(NODE_PREFIX "%zd [" NODE_PARAMS ", label=<<table cellspacing=\"0\">\n"
                                                           "<tr><td colspan=\"2\">phys idx = %zd </td></tr>\n", phys_i, phys_i);

        if (list->arr[phys_i].elem == ListNode::POISON) {
            FPRINTF_("<tr><td colspan=\"2\">elem = PZN</td></tr>\n");
        } else {
            FPRINTF_("<tr><td colspan=\"2\">elem = " ELEM_T_PRINTF "</td></tr>\n", list->arr[phys_i].elem);
        }

        FPRINTF_("<tr><td>prev = %zd </td><td>next = %zd</td></tr></table>>", list->arr[phys_i].prev, list->arr[phys_i].next);

        if (list->arr[phys_i].prev == -1)
            FPRINTF_(", color=yellow");

        FPRINTF_("];\n\n");
    }

    FPRINTF_("{rank=same;");
    for (ssize_t phys_i = 0; phys_i < list->capacity; phys_i++) {
        FPRINTF_(" " NODE_PREFIX "%zd", phys_i);
    }
    FPRINTF_("};\n");

    for (ssize_t phys_i = 0; phys_i < list->capacity; phys_i++) {
        if (phys_i != 0)
            FPRINTF_("->");

        FPRINTF_(NODE_PREFIX "%zd", phys_i);
    }
    FPRINTF_("[style=invis];\n\n");

    ssize_t phys_i = list_head(list);
    ssize_t log_i = 0;

    LIST_FOREACH(*list, phys_i, log_i) {
        if (list->arr[phys_i].next != 0)
            FPRINTF_(NODE_PREFIX "%zd->" NODE_PREFIX "%zd [color=green, weight=0];\n", phys_i, list->arr[phys_i].next);

        if (list->arr[phys_i].prev != 0)
            FPRINTF_(NODE_PREFIX "%zd->" NODE_PREFIX "%zd [color=blue, weight=0];\n", phys_i, list->arr[phys_i].prev);
    }

    phys_i = list->free_head;
    log_i = 0;
    // macros is not used because of different size clause
    for (; phys_i > 0 && log_i <= list->capacity - list->size;
           phys_i = list->arr[phys_i].next, log_i++) {

        if (list->arr[phys_i].next != 0)
            FPRINTF_(NODE_PREFIX "%zd->" NODE_PREFIX "%zd [color=yellow, weight=0];\n",
                                  phys_i,             list->arr[phys_i].next);
    }

    FPRINTF_("head [shape=rect, label=\"HEAD\", color=yellow, fillcolor=\"#7293ba\",style=filled];\n");
    FPRINTF_("tail [shape=rect, label=\"TAIL\", color=yellow, fillcolor=\"#7293ba\",style=filled];\n");
    FPRINTF_("free_head [shape=rect, label=\"FREE_HEAD\","
                        "color=yellow, fillcolor=\"#7293ba\", style=filled];\n");

    FPRINTF_("head->" NODE_PREFIX "%zd [color=yellow];\n", list_head(list));
    FPRINTF_("tail->" NODE_PREFIX "%zd [color=yellow];\n", list_tail(list));
    FPRINTF_("free_head->" NODE_PREFIX "%zd [color=yellow];\n", list->free_head);

    FPRINTF_("}\n");

    if (fclose(file) != 0) {
        perror("Error closing file");
        return false;
    }

    str_len = strncat_len(img_filename, log_file.timestamp_dir, log_file.MAX_FILENAME_LEN);
    snprintf(img_filename + str_len, log_file.MAX_FILENAME_LEN - str_len, "%zd", dot_number++);
    str_len = strncat_len(img_filename, ".svg", log_file.MAX_FILENAME_LEN);

    if (!create_img(dot_filename, img_filename)) {
        fprintf(stderr, "Error creating dot graph\n");
        return false;
    }

    return true;
}
#undef FPRINTF_

#endif //< #ifdef DEBUG

#define PRINT_ERR_(code, descr)  if ((err_code) & List::code)                                       \
                                    log_printf(&log_file,                                           \
                                               HTML_TEXT(HTML_RED("!!! " #code ": " descr "\n")));
void list_print_error(const int err_code) {
    if (err_code == List::OK) {
        log_printf(&log_file, HTML_TEXT(HTML_GREEN("No error\n")));
    } else {
        PRINT_ERR_(ALREADY_INITIALISED, "Constructor called for already initialised or corrupted list");
        PRINT_ERR_(UNITIALISED,         "List is not initialised");
        PRINT_ERR_(DATA_INVALID_PTR,    "list.arr pointer is not valid for writing");
        PRINT_ERR_(ALLOC_ERR,           "Can't allocate memory");
        PRINT_ERR_(POISON_VAL_FOUND,    "There is poison value in list");
        PRINT_ERR_(NON_POISON_EMPTY,    "Empty element is not poison");
        PRINT_ERR_(LOW_CAPACITY,        "list.size > list.capacity - 1");
        PRINT_ERR_(NEGATIVE_CAPACITY,   "Negative list.capacity");
        PRINT_ERR_(INVALID_CAPACITY,    "Invalid capacity given");
        PRINT_ERR_(NEGATIVE_SIZE,       "Negative list.size");
        PRINT_ERR_(INVALID_POSITION,    "Invalid physical index given");
        PRINT_ERR_(DAMAGED_PATH,        "List is damaged. Invalid path");
        PRINT_ERR_(INVALID_FREE_HEAD,   "Invalid free_head field");
        PRINT_ERR_(INVALID_TAIL,        "Invalid tail field");
        PRINT_ERR_(INVALID_HEAD,        "Invalid head field");
        PRINT_ERR_(INVALID_IS_LINEAR,   "is_linear flag is true, but list isn't linear");
    }
}
#undef PRINT_ERR_
