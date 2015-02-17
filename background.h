//
//  background.h
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#ifndef smsh_background_h
#define smsh_background_h

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "list.h"

struct background_item {
	pid_t process_id;
	char *command;
};

struct list *background_create();
void background_free(struct list *backgrounds);

struct background_item *background_create_item(pid_t process_id, char *command);
void background_free_item(struct background_item *item);

void background_check(struct list *backgrounds);

#endif
