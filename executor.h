//
//  executor.h
//  smsh
//
//  Created by Jonghwan Hyeon on 12/3/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#ifndef smsh_executor_h
#define smsh_executor_h

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "utility.h"
#include "list.h"
#include "parser.h"
#include "history.h"
#include "background.h"

void executor_execute_commands(struct list *commands);

#endif
