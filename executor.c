//
//  executor.c
//  smsh
//
//  Created by Jonghwan Hyeon on 12/3/13.
//  Copyright (c) 2013 Jonghwan Hyeon. All rights reserved.
//

#include "executor.h"

int executor_execute_command(struct parser_command *command);

int executor_execute_normal_command(struct list_iterator *iterator);

int executor_execute_pipe_commands(struct list *commands);
int executor_do_execute_pipe_commands(struct list_iterator *traversal_iterator, struct list_iterator *execution_iterator, int previous_pipe_descriptors[2]);

int executor_execute_built_in_command(struct list_iterator *iterator);
int executor_execute_change_directory(struct parser_command *command);
int executor_execute_set_shell_variable(struct parser_command *command);
int executor_execute_unset_shell_variable(struct parser_command *command);
int executor_execute_history(struct parser_command *command);

struct list *excutor_extract_pipe_commands(struct list_iterator *iterator);

int executor_open_from_redirection(struct parser_redirection *redirection);
int executor_apply_redirection(struct parser_command *command);

int executor_is_built_in_command(struct parser_command *command);

int noclobber;
extern struct list *histories;
extern struct list *backgrounds;

int executor_execute_command(struct parser_command *command) {
    if (executor_apply_redirection(command) < 0) {
        return -1;
    }

    if (strcmp(command->filename, "history") == 0) {
        executor_execute_history(command);
        exit(0);
    } else {
        if (execvp(command->filename, (char **)list_to_array(command->parameters)) < 0) {
            fprintf(stderr, "%s: Command not found.\n", command->filename);
            fflush(stderr);
            
            exit(-1);
        }
    }
    
    return 0;
}

void executor_execute_commands(struct list *commands) {
    struct list_iterator *iterator = list_first_iterator(commands);
    while (list_iterator_has_next(iterator)) {
        struct parser_command *command = list_iterator_get_next(iterator);
        
        if (executor_is_built_in_command(command)) {
            executor_execute_built_in_command(iterator);
        } else {
            if (command->type == PARSER_COMMAND_PIPE) {
                struct list *pipe_commands = excutor_extract_pipe_commands(iterator);
                
                if (executor_execute_pipe_commands(pipe_commands) < 0) {
                    list_free(pipe_commands);
                    break ;
                }
                
                list_free(pipe_commands);
            } else {
                if (executor_execute_normal_command(iterator) < 0) {
                    break ;
                }
            }
        }
    }
    
    list_iterator_free(iterator);
}

int executor_execute_normal_command(struct list_iterator *iterator) {
    struct parser_command *command = list_iterator_next(iterator);
    
    pid_t process_id = fork();
    if (process_id < 0) return -1;
    
    if (process_id == 0) {
        executor_execute_command(command);
    } else {
        if (command->type == PARSER_COMMAND_BACKGROUND) {
            list_push(backgrounds, background_create_item(process_id, parser_stringify_command(command)));
            printf("[%d] %d\n", list_length(backgrounds), process_id);
            fflush(stdout);
            
            return 0;
        } else {
            int status;
            waitpid(process_id, &status, 0);
            return status;
        }
    }
    
    return -1;
}

int executor_execute_pipe_commands(struct list *commands) {
    struct list_iterator *traversal_iterator = list_first_iterator(commands);
    struct list_iterator *execution_iterator = list_last_iterator(commands);
    
    int result = executor_do_execute_pipe_commands(traversal_iterator, execution_iterator, NULL);
    
    list_iterator_free(execution_iterator);
    list_iterator_free(traversal_iterator);
    
    return result;
}

int executor_do_execute_pipe_commands(struct list_iterator *traversal_iterator, struct list_iterator *execution_iterator, int previous_pipe_descriptors[2]) {
    
    if (!list_iterator_has_next(traversal_iterator)) return 0;
    
    pid_t process_id = fork();
    if (process_id < 0) return -1;
    
    struct parser_command *traversal_command = list_iterator_next(traversal_iterator);
    struct parser_command *execution_command = list_iterator_previous(execution_iterator);
    
    if (process_id == 0) {
        
        if (traversal_command->type == PARSER_COMMAND_PIPE) {
            if (traversal_command->in_redirection->type != PARSER_REDIRECTION_NONE) {
                fprintf(stderr, "Ambiguous input redirect.\n");
                fflush(stderr);
                return -1;
            }
            
            if (traversal_command->out_redirection->type != PARSER_REDIRECTION_NONE) {
                fprintf(stderr, "Ambiguous output redirect.\n");
                fflush(stderr);
                return -1;
            }
            
            int pipe_descriptors[2];
            pipe(pipe_descriptors);
            
            if (executor_do_execute_pipe_commands(traversal_iterator, execution_iterator, pipe_descriptors) < 0) {
                return -1;
            }
            
            close(STDIN_FILENO);
            dup(pipe_descriptors[0]);
            close(pipe_descriptors[0]);
            close(pipe_descriptors[1]);
        }
        
        if (previous_pipe_descriptors != NULL) {
            close(STDOUT_FILENO);
            dup(previous_pipe_descriptors[1]);
            close(previous_pipe_descriptors[0]);
            close(previous_pipe_descriptors[1]);
        }
        
        executor_execute_command(execution_command);
    }
    
    if (traversal_command->type == PARSER_COMMAND_BACKGROUND) {
        list_push(backgrounds, background_create_item(process_id, parser_stringify_command(execution_command)));
        printf("[%d] %d\n", list_length(backgrounds), process_id);
        fflush(stdout);
        
        return 0;
    } else {
        int status;
        wait(&status);
        
        return status;
    }
    
    return 0;
}

int executor_execute_built_in_command(struct list_iterator *iterator) {
    struct parser_command *command = list_iterator_next(iterator);
    
    if (strcmp(command->filename, "cd") == 0) {
        return executor_execute_change_directory(command);
    } else if (strcmp(command->filename, "set") == 0) {
        return executor_execute_set_shell_variable(command);
    } else if (strcmp(command->filename, "unset") == 0) {
        return executor_execute_unset_shell_variable(command);
    }
    
    return -1;
}

int executor_execute_change_directory(struct parser_command *command) {
    char *path = getenv("HOME");
    if (list_length(command->parameters) > 2) path = list_get(command->parameters, 1);
    
    if (chdir(path) < 0) {
        fprintf(stderr, "%s: No such file or directory.\n", path);
        fflush(stderr);
        
        return -1;
    }
    
    return 0;
}

int executor_execute_set_shell_variable(struct parser_command *command) {
    if (list_length(command->parameters) == 2) {
        printf("%s\t%d\n", "noclobber", noclobber);
        fflush(stdout);
    } else {
        char *variable = list_get(command->parameters, 1);
        if (strcmp(variable, "noclobber") == 0) {
            noclobber = 1;
        } else {
            fprintf(stderr, "%s: Not supported shell variable.\n", variable);
            fflush(stderr);
            
            return -1;
        }
    }
    
    return 0;
}

int executor_execute_unset_shell_variable(struct parser_command *command) {
    if (list_length(command->parameters) == 2) {
        fprintf(stderr, "unset: Too few arguments.\n");
        fflush(stderr);
        
        return -1;
    } else {
        char *variable = list_get(command->parameters, 1);
        if (strcmp(variable, "noclobber") == 0) {
            noclobber = 0;
        } else {
            fprintf(stderr, "%s: Not supported shell variable.\n", variable);
            fflush(stderr);
            
            return -1;
        }
    }
    
    return 0;
}

int executor_execute_history(struct parser_command *command) {
    int count = 0;
    if (list_length(command->parameters) > 2) {
        count = atoi(list_get(command->parameters, 1));
    }
    
    return history_print(histories, count);
}

struct list *excutor_extract_pipe_commands(struct list_iterator *iterator) {
    struct list *commands = list_create();
    
    while (list_iterator_has_next(iterator)) {
        struct parser_command *command = list_iterator_next(iterator);
        
        list_push(commands, command);
        if (command->type != PARSER_COMMAND_PIPE) break ;
    }
    
    return commands;
}

int executor_apply_redirection(struct parser_command *command) {
    int in_descriptor;
    int out_descriptor;
    
    if (command->in_redirection->type != PARSER_REDIRECTION_NONE) {
        printf("in direction\n"); fflush(stdout);
        if (command->type == PARSER_COMMAND_PIPE) {
            fprintf(stderr, "Ambiguous intput redirect.\n");
            fflush(stderr);
            return -1;
        }
        
        in_descriptor = executor_open_from_redirection(command->in_redirection);
        if (in_descriptor < 0) {
            return -1;
        }
        
        close(STDIN_FILENO);
        dup(in_descriptor);
        close(in_descriptor);
    }
    
    if (command->out_redirection->type != PARSER_REDIRECTION_NONE) {
        printf("out direction\n"); fflush(stdout);
        if (command->type == PARSER_COMMAND_PIPE) {
            fprintf(stderr, "Ambiguous output redirect.\n");
            fflush(stderr);
            return -1;
        }
        
        out_descriptor = executor_open_from_redirection(command->out_redirection);
        if (out_descriptor < 0) {
            return -1;
        }
        
        close(STDOUT_FILENO);
        dup(out_descriptor);
        close(out_descriptor);
    }
    
    return 0;
}

int executor_open_from_redirection(struct parser_redirection *redirection) {
    int descriptor;
    
    switch (redirection->type) {
        case PARSER_REDIRECTION_STANDARD_IN:
            descriptor = open(redirection->filename, O_RDONLY);
            break;
            
        case PARSER_REDIRECTION_TRUNCATING_STANDARD_OUT:
            if (noclobber && file_exists(redirection->filename)) {
                fprintf(stderr, "%s: File exists.\n", redirection->filename);
                fflush(stderr);
                return -1;
            }
            descriptor = open(redirection->filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            break;
            
        case PARSER_REDIRECTION_APPENDING_STANDARD_OUT:
            if (noclobber && file_exists(redirection->filename)) {
                fprintf(stderr, "%s: File exists.\n", redirection->filename);
                fflush(stderr);
                return -1;
            }
            descriptor = open(redirection->filename, O_CREAT | O_APPEND | O_WRONLY, 0644);
            break;
            
        case PARSER_REDIRECTION_OVERWRITING_STANDARD_OUT:
            descriptor = open(redirection->filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            break;
            
        default:
            descriptor = -1;
            break;
    }
    
    return descriptor;
}

int executor_is_built_in_command(struct parser_command *command) {
    return (strcmp(command->filename, "cd") == 0)
        || (strcmp(command->filename, "set") == 0)
        || (strcmp(command->filename, "unset") == 0);
}

