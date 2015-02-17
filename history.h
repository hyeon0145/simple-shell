//
//  history.h
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#ifndef smsh_history_h
#define smsh_history_h

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "utility.h"
#include "list.h"

struct history {
    char *command;
    time_t time;
};

struct list *history_create();
void history_free(struct list *histories);

struct history *history_create_item(char *command);
void history_free_item(struct history *history);

int history_print(struct list *histories, int count);

#endif
