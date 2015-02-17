//
//  preprocessor.c
//  smsh
//
//  Created by Jonghwan Hyeon on 12/4/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#include "preprocessor.h"

int preprocessor_preprocess_single_quote(struct preprocessor_data *data);
int preprocessor_preprocess_double_quote(struct preprocessor_data *data);
int preprocessor_preprocess_history(struct preprocessor_data *data);

extern struct list *histories;

int preprocessor_preprocess(char *command) {
    struct preprocessor_data data;
    data.command = command;
    data.current_command = &command[0];
    data.current_buffer = &data.buffer[0];
    
    while (*data.current_command != '\0') {
        if (*data.current_command == '\'') {
            if (preprocessor_preprocess_single_quote(&data) < 0) return -1;
        } else if (*data.current_command == '"') {
            if (preprocessor_preprocess_double_quote(&data) < 0) return -1;
        } else if (*data.current_command == '!') {
            if (preprocessor_preprocess_history(&data) < 0) return -1;
        } else {
            *data.current_buffer = *data.current_command;
            data.current_buffer++;
            data.current_command++;
        }
    }
    
    *data.current_buffer = '\0';
    strcpy(command, data.buffer);
    
    return 0;
}


int preprocessor_preprocess_single_quote(struct preprocessor_data *data) {
    *data->current_buffer = *data->current_command;
    data->current_buffer++;
    data->current_command++;
    
    while (1) {
        if (*data->current_command == '\0') {
            fprintf(stderr, "Unmatched '.\n");
            fflush(stderr);
            
            return -1;
        }
        
        if (*data->current_command == '\'') break;
        
        *data->current_buffer = *data->current_command;
        data->current_buffer++;
        
        data->current_command++;
    }
    
    *data->current_buffer = *data->current_command;
    data->current_buffer++;
    data->current_command++;
    
    return 0;
}

int preprocessor_preprocess_double_quote(struct preprocessor_data *data) {
    *data->current_buffer = *data->current_command;
    data->current_buffer++;
    data->current_command++;
    
    while (1) {
        if (*data->current_command == '\0') {
            fprintf(stderr, "Unmatched \".\n");
            fflush(stderr);
            
            return -1;
        }
        
        if (*data->current_command == '"') break;
        
        *data->current_buffer = *data->current_command;
        data->current_buffer++;
        
        data->current_command++;
    }
    
    *data->current_buffer = *data->current_command;
    data->current_buffer++;
    data->current_command++;
    
    return 0;
}

int preprocessor_preprocess_history(struct preprocessor_data *data) {
    data->current_command++;
    
    int number = 0;
    struct history *history;
    if (*data->current_command == '!') {
        data->current_command++;
    } else if (isdigit(*data->current_command)) {
        number = atoi(data->current_command);
        while (isdigit(*data->current_command)) {
            data->current_command++;
        }
    } else {
        *data->current_buffer = *(data->current_command - 1);
        data->current_buffer++;
        
        return 0;
    }
    
    if (((number == 0) && (list_length(histories) == 0)) || (number > list_length(histories))) {
        fprintf(stderr, "%d: Event not found.\n", number);
        fflush(stderr);
        
        return -1;
    }
    
    if (number == 0) number = list_length(histories);
    history = list_get(histories, number - 1);
    strcpy(data->current_buffer, history->command);
    data->current_buffer = data->current_buffer + strlen(history->command);
    
    return 0;
}