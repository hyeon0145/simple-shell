//
//  history.c
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#include "history.h"

struct list *history_create() {
    struct list *histories = list_create();
    
    return histories;
}

void history_free(struct list *histories) {
    struct list_iterator *iterator = list_first_iterator(histories);
    while (list_iterator_has_next(iterator)) {
        history_free_item(list_iterator_next(iterator));
    }
    list_iterator_free(iterator);
    
    list_free(histories);
}

struct history *history_create_item(char *command) {
    struct history *history = malloc(sizeof(struct history));
    history->command = strdup(command);
    history->time = time(NULL);
    
    return history;
}

void history_free_item(struct history *history) {
    free(history->command);
    free(history);
}

int history_print(struct list *histories, int count) {
    if (count < 0) {
        fprintf(stderr, "Usage: history [# number of events].\n");
        fflush(stderr);
        
        return -1;
    }
    
    int current_index = 0;
    
    struct list_iterator *iterator = list_first_iterator(histories);
    if (count != 0) {
        int forward_count = list_length(histories) - count;
        if (forward_count > 0) {
            for (int i = 0; i < forward_count; i++) {
                list_iterator_next(iterator);
                current_index++;
            }
        }
    }
    
    while (list_iterator_has_next(iterator)) {
        struct history *history = list_iterator_next(iterator);
        struct tm *time = localtime(&history->time);
        
        printf("\t%d\t%d:%d\t%s\n", current_index + 1, time->tm_hour, time->tm_min, history->command);
        fflush(stdout);
        
        current_index++;
    }
    list_iterator_free(iterator);
    
    return 0;
}