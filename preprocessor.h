//
//  preprocessor.h
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#ifndef smsh_preprocessor_h
#define smsh_preprocessor_h

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "list.h"
#include "history.h"

struct preprocessor_data {
	char *command;
	char *current_command;
    
	char buffer[1024];
	char *current_buffer;
};


int preprocessor_preprocess(char *command);

#endif
