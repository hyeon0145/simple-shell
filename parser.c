#include "parser.h"

struct list *parser_parse(struct list *tokens, int *error) {
    struct list *sequence_of_commands_list = list_create();
    
    struct parser_data data;
    data.tokens = tokens;
    data.iterator = list_first_iterator(tokens);
    data.error = 0;
    
    if (error != NULL) *error = 0;
    
    struct list *commands = list_create();
    while (list_iterator_has_next(data.iterator)) {
        data.command = parser_create_command();

        parser_handle_command(&data);
        
        if (data.error != 0) {
            if (error != NULL) *error = data.error;
            list_iterator_free(data.iterator);
            return NULL;
        }
        
        list_push(commands, data.command);
    }
    list_iterator_free(data.iterator);
    
    struct list_iterator *iterator = list_first_iterator(commands);
    while (list_iterator_has_next(iterator)) {
        list_push(sequence_of_commands_list, parser_get_sequence_of_commands(iterator));
    }
    list_iterator_free(iterator);
    list_free(commands);
    
    return sequence_of_commands_list;
}

void parser_free(struct list *sequence_of_commands_list) {
    struct list_iterator *sequence_of_commands_list_iterator = list_first_iterator(sequence_of_commands_list);
    while (list_iterator_has_next(sequence_of_commands_list_iterator)) {
        struct list *sequence_of_commands = list_iterator_next(sequence_of_commands_list_iterator);
        
        struct list_iterator *sequence_of_commands_iterator = list_first_iterator(sequence_of_commands);
        while (list_iterator_has_next(sequence_of_commands_iterator)) {
            parser_free_command(list_iterator_next(sequence_of_commands_iterator));
        }
        
        list_iterator_free(sequence_of_commands_iterator);
        list_free(sequence_of_commands);
    }
    
    list_iterator_free(sequence_of_commands_list_iterator);
    list_free(sequence_of_commands_list);
}

struct parser_command *parser_create_command() {
    struct parser_command *command = malloc(sizeof(struct parser_command));
    
    command->type = PARSER_COMMAND_NONE;
    
    command->filename = NULL;
    command->parameters = list_create();
    
    command->in_redirection = parser_create_redirection();
    command->out_redirection = parser_create_redirection();
    
    return command;
}

void parser_free_command(struct parser_command *command) {
    free(command->filename);
    
    struct list_iterator *iterator = list_first_iterator(command->parameters);
    while (list_iterator_has_next(iterator)) {
        free(list_iterator_next(iterator));
    }
    list_iterator_free(iterator);
    list_free(command->parameters);
    
    parser_free_redirection(command->in_redirection);
    parser_free_redirection(command->out_redirection);
    
    free(command);
}

struct parser_redirection *parser_create_redirection() {
    struct parser_redirection *redirection = malloc(sizeof(struct parser_redirection));
    redirection->type = PARSER_REDIRECTION_NONE;
    redirection->filename = NULL;
    
    return redirection;
}

void parser_free_redirection(struct parser_redirection *redirection) {
    free(redirection->filename);
    free(redirection);
}

void parser_handle_command(struct parser_data *data) {
    struct tokenizer_token *token = list_iterator_next(data->iterator);
    
    if (parser_is_word(token)) {
        parser_handle_filename(data);
    } else {
        data->error = 999;
    }
}

void parser_handle_filename(struct parser_data *data) {
    struct tokenizer_token *token = list_iterator_get(data->iterator);
    
    data->command->filename = strdup(token->data);
    list_push(data->command->parameters, strdup(token->data));
    
    parser_handle_parameter_and_redirector(data);
}


void parser_handle_parameter_and_redirector(struct parser_data *data) {
    while (list_iterator_has_next(data->iterator)) {
        struct tokenizer_token *token = list_iterator_next(data->iterator);
        
        if (parser_is_parameter_token(token)) {
            parser_handle_parameter(data);
        } else if (parser_is_redirector_token(token)) {
            parser_handle_redirector(data);
        } else if (parser_is_separator_token(token)) {
            list_iterator_previous(data->iterator);
            break;
        }
        
        if (data->error != 0) return ;
    }
    
    parser_handle_separator(data);
}

void parser_handle_parameter(struct parser_data *data) {
    struct tokenizer_token *token = list_iterator_get(data->iterator);
    
    list_push(data->command->parameters, strdup(token->data));
}

void parser_handle_redirector(struct parser_data *data) {
    struct tokenizer_token *token = list_iterator_get(data->iterator);
    
    struct parser_redirection *redirection = data->command->in_redirection;
    if (token->data[0] == '>') redirection = data->command->out_redirection;
    if (redirection->type != PARSER_REDIRECTION_NONE) {
        data->error = 100;
        if (token->data[0] == '>') data->error = 101;
    }
    
    if (strcmp(token->data, "<") == 0) {
        redirection->type = PARSER_REDIRECTION_STANDARD_IN;
    } else if (strcmp(token->data, ">") == 0) {
        redirection->type = PARSER_REDIRECTION_TRUNCATING_STANDARD_OUT;
    } else if (strcmp(token->data, ">>") == 0) {
        redirection->type = PARSER_REDIRECTION_APPENDING_STANDARD_OUT;
    } else if (strcmp(token->data, ">!") == 0) {
        redirection->type = PARSER_REDIRECTION_OVERWRITING_STANDARD_OUT;
    }
    
    if (!list_iterator_has_next(data->iterator)) {
        data->error = 102;
        return ;
    }
    
    token = list_iterator_next(data->iterator);
    redirection->filename = strdup(token->data);
}

void parser_handle_separator(struct parser_data *data) {
    list_push(data->command->parameters, NULL); // for the convenience when calling exec
    
    if (!list_iterator_has_next(data->iterator)) {
        data->command->type = PARSER_COMMAND_NORMAL;
        return ;
    }
    
    struct tokenizer_token *token = list_iterator_next(data->iterator);
    if (strcmp(token->data, ";") == 0) {
        data->command->type = PARSER_COMMAND_NORMAL;
    } else if (strcmp(token->data, "&") == 0) {
        data->command->type = PARSER_COMMAND_BACKGROUND;
    } else if (strcmp(token->data, "|") == 0) {
        data->command->type = PARSER_COMMAND_PIPE;
    } else if (strcmp(token->data, "&&") == 0) {
        data->command->type = PARSER_COMMAND_AND;
    } else if (strcmp(token->data, "||") == 0) {
        data->command->type = PARSER_COMMAND_OR;
    }
}

struct list *parser_get_sequence_of_commands(struct list_iterator *iterator) {
    struct list *sequence_of_commands = list_create();
    
    while (list_iterator_has_next(iterator)) {
        struct parser_command *command = list_iterator_next(iterator);
        
        list_push(sequence_of_commands, command);
        
        if (command->type == PARSER_COMMAND_NORMAL) break ;
    }
    
    return sequence_of_commands;
}

char *parser_stringify_command(struct parser_command *command) {
    char buffer[1024];
    strcpy(buffer, command->filename);

    struct list_iterator *iterator = list_first_iterator(command->parameters);
    list_iterator_next(iterator);
    
    while (list_iterator_has_next(iterator)) {
        char *parameter = list_iterator_next(iterator);
        if (parameter == NULL) break ;
        
        strcat(buffer, " ");
        strcat(buffer, parameter);
    }
    list_iterator_free(iterator);

    if (command->in_redirection->type != PARSER_REDIRECTION_NONE) {
        strcat(buffer, " < ");
        strcat(buffer, command->in_redirection->filename);
    }

    if (command->out_redirection->type != PARSER_REDIRECTION_NONE) {
        if (command->out_redirection->type == PARSER_REDIRECTION_TRUNCATING_STANDARD_OUT) {
            strcat(buffer, " > ");
        } else if (command->out_redirection->type == PARSER_REDIRECTION_APPENDING_STANDARD_OUT) {
            strcat(buffer, " >> ");
        } else {
            strcat(buffer, " >! ");
        }

        strcat(buffer, command->out_redirection->filename);
    }

    return strdup(buffer);
}

int parser_is_redirector_token(struct tokenizer_token *token) {
    if (token->type == TOKENIZER_TOKEN_SINGLE_SPECIAL_CHARACTER) {
        return (strchr("<>", token->data[0]) != NULL);
    } else if (token->type == TOKENIZER_TOKEN_DOUBLE_SPECIAL_CHARACTERS) {
        return (
            (strcmp(token->data, ">>") == 0)
            || (strcmp(token->data, "<<") == 0)
            || (strcmp(token->data, ">!") == 0)
        );
    }
    
    return 0;
}

int parser_is_separator_token(struct tokenizer_token *token) {
    if (token->type == TOKENIZER_TOKEN_SINGLE_SPECIAL_CHARACTER) {
        return (strchr(";&|", token->data[0]) != NULL);
    } else if (token->type == TOKENIZER_TOKEN_DOUBLE_SPECIAL_CHARACTERS) {
        return (
            (strcmp(token->data, "&&") == 0)
            || (strcmp(token->data, "||") == 0)
        );
    }
    
    return 0;
}

char *parser_error_message(int error) {
    switch (error) {
        case 100:
            return "Ambiguous input redirect.";
            break;
        case 101:
            return "Ambiguous output redirect.";
            break;
        case 102:
            return "Missing name for redirect.";
            break;
        default:
            return "Unknown error.";
            break;
    }
}
