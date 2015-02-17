//
//  background.c
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#include "background.h"

void background_truncate(struct list *backgrounds);
char *background_name_of_signal(int signal);

struct list *background_create() {
    struct list *backgrounds = list_create();
    
    return backgrounds;
}

void background_free(struct list *backgrounds) {
    struct list_iterator *iterator = list_first_iterator(backgrounds);
    while (list_iterator_has_next(iterator)) {
        background_free_item(list_iterator_next(iterator));
    }
    list_iterator_free(iterator);

    list_free(backgrounds);
}

struct background_item *background_create_item(pid_t process_id, char *command) {
    struct background_item *item = (struct background_item *)malloc(sizeof(struct background_item));
    item->process_id = process_id;
    item->command = command;

    return item;
}

void background_free_item(struct background_item *item) {
    free(item->command);
    free(item);
}

void background_check(struct list *backgrounds) {
    int index = 1;
    
    struct list_iterator *iterator = list_first_iterator(backgrounds);
    while (list_iterator_has_next(iterator)) {
        struct background_item *item = list_iterator_next(iterator);
        int status;
        
        waitpid(item->process_id, &status, WNOHANG);
        if (WIFEXITED(status)) {
            item->process_id = 0;
            printf("[%d]\tDone\t%s\n", index, item->command);
        } else if (WIFSIGNALED(status)) {
            item->process_id = 0;
            printf("[%d]\t%s\n", index, background_name_of_signal(WTERMSIG(status)));
        }
        fflush(stdout);
        
        index++;
    }
    list_iterator_free(iterator);
    
    background_truncate(backgrounds);
}


void background_truncate(struct list *backgrounds) {
    struct list_iterator *iterator = list_first_iterator(backgrounds);
    
    iterator = list_last_iterator(backgrounds);
    while (list_iterator_has_previous(iterator)) {
        struct background_item *item = list_iterator_previous(iterator);
        if (item->process_id == 0) list_iterator_remove(iterator);
    }
    list_iterator_free(iterator);
}


char *background_name_of_signal(int signal) {
    switch (signal) {
        case SIGHUP:
            return "Hangup";
            break;
        case SIGINT:
            return "Interrupt";
            break;
        case SIGQUIT:
            return "Quit";
            break;
        case SIGILL:
            return "Illegal instruction";
            break;
        case SIGABRT:
            return "Abort";
            break;
        case SIGFPE:
            return "Floating exception";
            break;
        case SIGKILL:
            return "Killed";
            break;
        case SIGSEGV:
            return "Segmentation fault";
            break;
        case SIGPIPE:
            return "Broken pipe";
            break;
        case SIGALRM:
            return "Alarm clock";
            break;
        case SIGTERM:
            return "Terminated";
            break;
        default:
            return "Signaled";
            break;
    }
}