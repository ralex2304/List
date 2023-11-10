#include "log/log.h"
#include "list.h"

LogFileData log_file = {"log"};

int main() {
    log_open_file(&log_file, "wb");

    List list = {};
    LIST_CTOR(&list);

    size_t index = 0;
    LIST_DUMP(&list);

    list_insert_after(&list, 0, 0, &index);
    list_insert_after(&list, 1, 10, &index);
    list_insert_after(&list, 2, 20, &index);

    LIST_DUMP(&list);

    list_insert_after(&list, 2, 15, &index);

    LIST_DUMP(&list);

    list_insert_after(&list, 2, 20, &index);

    LIST_DUMP(&list);

    list_insert_after(&list, 2, 50, &index);
    list_insert_after(&list, 2, 60, &index);
    list_insert_after(&list, 2, 70, &index);
    list_insert_after(&list, 8, 80, &index);
    list_insert_after(&list, 9, 90, &index);

    LIST_DUMP(&list);

    list_dtor(&list);

    log_close_file(&log_file);
}
