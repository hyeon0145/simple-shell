#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

#include "utility.h"

#include "list.h"
#include "tokenizer.h"
#include "parser.h"

#include "history.h"
#include "background.h"
#include "preprocessor.h"
#include "executor.h"

#define SIZE_OF_BUFFER 1024

char *prompt(char *buffer, size_t size);

void print_error(char *message);

struct list *histories;
struct list *backgrounds;

int main(int argc, char *argv[]) {
    char buffer[SIZE_OF_BUFFER];
    
    histories = history_create();
    backgrounds = background_create();
    
    while (1) {

        if (prompt(buffer, SIZE_OF_BUFFER - 1) == NULL) {
            continue ;
        }

        if (buffer[0] == '\0') {
            background_check(backgrounds);
            continue ;
        } else if (strcmp(buffer, "exit") == 0) {
            break ;
        }
        
        if (preprocessor_preprocess(buffer) < 0) {
            continue ;
        }
        
        background_check(backgrounds);
        
        list_push(histories, history_create_item(buffer));
        
        int error;
        struct list *tokens = tokenizer_tokenize(buffer, &error);
        if (error != 0) {
            print_error(tokenizer_error_message(error));
            continue ;
        }
        
        struct list *sequence_of_commands_list = parser_parse(tokens, &error);
        if (error != 0) {
            print_error(parser_error_message(error));
            continue ;
        }
        
        struct list_iterator *iterator = list_first_iterator(sequence_of_commands_list);
        while (list_iterator_has_next(iterator)) {
            struct list *sequence_of_commands = list_iterator_next(iterator);
            executor_execute_commands(sequence_of_commands);
        }
        list_iterator_free(iterator);
        
        parser_free(sequence_of_commands_list);
        tokenizer_free(tokens);
    }
    
    background_free(backgrounds);
    history_free(histories);
    
	return 0;
}

char *prompt(char *buffer, size_t size) {
    char prompt[SIZE_OF_BUFFER];
    get_prompt(prompt, SIZE_OF_BUFFER - 1);
    write(STDOUT_FILENO, prompt, strlen(prompt));
    
    ssize_t read_bytes;
    buffer[0] = '\0';
    read_bytes = read(STDIN_FILENO, buffer, size - 1);
    if (read_bytes < 0) {
        perror("prompt error: ");
        return NULL;
    }
    
    /* remove trailing new line character and make this to null-terminated string */
    buffer[read_bytes - 1] = '\0';
    
    return buffer;
}

void print_error(char *message) {
    size_t length_of_message = strlen(message);
    
    write(STDERR_FILENO, message, length_of_message);
    if (message[length_of_message - 1] != '\n') {
        write(STDERR_FILENO, "\n", 1);
    }
}