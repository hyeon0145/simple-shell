#include "tokenizer.h"

struct list *tokenizer_tokenize(char *source, int *error) {
    struct list *tokens = list_create();

    struct tokenizer_data data;
    data.source = source;
    data.current_source = source;
    data.error = 0;
    
    if (error != NULL) *error = 0;
    
    while (*data.current_source != '\0') {
        data.current_token = &data.token[0];
        data.type = TOKENIZER_TOKEN_NONE;

        tokenizer_handle_ready(&data);
        if (data.error != 0) {
            if (error != NULL) *error = data.error;

            return NULL;
        }

        if (data.current_token != &data.token[0]) {
            *data.current_token = '\0';
            
            struct tokenizer_token *token = tokenizer_create_token();
            token->data = strdup(data.token);
            token->type = data.type;

            list_push(tokens, token);
        }
    }

    return tokens;
}


void tokenizer_free(struct list *tokens) {
    struct list_iterator *iterator = list_first_iterator(tokens);
    while (list_iterator_has_next(iterator)) {
        tokenizer_free_token(list_iterator_next(iterator));
    }
    list_iterator_free(iterator);

    list_free(tokens);
}

struct tokenizer_token *tokenizer_create_token(char *data, enum TOKENIZER_TOKEN type) {
    struct tokenizer_token *token = malloc(sizeof(struct tokenizer_token));
    token->data = NULL;
    token->type = TOKENIZER_TOKEN_NONE;

    return token;
}

void tokenizer_free_token(struct tokenizer_token *token) {
    free(token->data);
    free(token);
}

void tokenizer_handle_ready(struct tokenizer_data *data) {
    if (tokenizer_is_white_space(*data->current_source)) {
        tokenizer_handle_white_space(data);
    } else if (tokenizer_is_word(*data->current_source)) {
        tokenizer_handle_simple_word(data);
    } else if (tokenizer_is_escape_character(*data->current_source)) {
        tokenizer_handle_escape_sequence(data);
    } else if (tokenizer_is_quote(*data->current_source)) {
        tokenizer_handle_quoted_word(data);
    } else if (tokenizer_is_single_special_character(*data->current_source)) {
        tokenizer_handle_single_special_character(data);
    } else if (tokenizer_is_double_possible_special_character(*data->current_source)) {
        tokenizer_handle_double_possible_special_character(data);
    } else {
        data->error = 999; /* Unknown error */
    }
}

void tokenizer_handle_white_space(struct tokenizer_data *data) {
    while (tokenizer_is_white_space(*data->current_source)) {
        data->current_source++;
    }

    data->type = TOKENIZER_TOKEN_WHITE_SPACE;
}

void tokenizer_handle_simple_word(struct tokenizer_data *data) {
    while (1) {
        if (tokenizer_is_word(*data->current_source)) {
            *data->current_token = *data->current_source;
            data->current_token++;

            data->current_source++;
        } else if (tokenizer_is_escape_character(*data->current_source)) {
            tokenizer_handle_escape_sequence(data);
        } else {
            break ;
        }
    }
    
    if (data->token[0] == '~') {
        char buffer[1024];
        
        *data->current_token = '\0';
        strcpy(buffer, getenv("HOME"));
        strcat(buffer, data->token + 1);
        
        strcpy(data->token, buffer);
        data->current_token = data->token + strlen(buffer);
    }
    
    data->type = TOKENIZER_TOKEN_SIMPLE_WORD;
}

void tokenizer_handle_escape_sequence(struct tokenizer_data *data) {
    data->current_source++; // escape character

    *data->current_token = *data->current_source;
    data->current_token++;

    data->current_source++;

    tokenizer_handle_simple_word(data);
}

void tokenizer_handle_quoted_word(struct tokenizer_data *data) {
    if (tokenizer_is_single_quote(*data->current_source)) {
        tokenizer_handle_single_quoted_word(data);        
    } else if (tokenizer_is_double_quote(*data->current_source)) {
        tokenizer_handle_double_quoted_word(data);        
    }

    data->type = TOKENIZER_TOKEN_QUOTED_WORD;
}

void tokenizer_handle_single_quoted_word(struct tokenizer_data *data) {
    data->current_source++; // single quote
    
    while (1) {
        if (*data->current_source == '\0') {
            data->error = 100;
            return ;
        }
        
        if (tokenizer_is_single_quote(*data->current_source)) break ;
        
        *data->current_token = *data->current_source;
        data->current_token++;
        
        data->current_source++;
    }
    
    data->current_source++; // single quote
}

void tokenizer_handle_double_quoted_word(struct tokenizer_data *data) {
    data->current_source++; // double quote

    while (1) {
        if (*data->current_source == '\0') {
            data->error = 101;
            return ;
        }
        
        if (tokenizer_is_double_quote(*data->current_source)) break ;
        
        *data->current_token = *data->current_source;
        data->current_token++;
        
        data->current_source++;
    }
    
    data->current_source++; // double quote
}

void tokenizer_handle_single_special_character(struct tokenizer_data *data) {
    *data->current_token = *data->current_source;
    data->current_token++;

    data->current_source++;

    data->type = TOKENIZER_TOKEN_SINGLE_SPECIAL_CHARACTER;
}

void tokenizer_handle_double_possible_special_character(struct tokenizer_data *data) {
    if (tokenizer_is_double_possible_special_character(*(data->current_source + 1))) {
        tokenizer_handle_double_special_character(data);
    } else {
        tokenizer_handle_single_special_character(data);
    }
}

void tokenizer_handle_double_special_character(struct tokenizer_data *data) {
    for (int i = 1; i <= 2; i++) {
        *data->current_token = *data->current_source;
        data->current_token++;
        
        data->current_source++;
    }

    data->type = TOKENIZER_TOKEN_DOUBLE_SPECIAL_CHARACTERS;
}

int tokenizer_is_word(char character){
    return (character != '\0') && (
        (!tokenizer_is_special_character(character)) 
        && (!tokenizer_is_quote(character)) 
        && (!tokenizer_is_white_space(character)) 
        && (!tokenizer_is_escape_character(character))
    );
}

int tokenizer_is_single_special_character(char character){
    return (character != '\0') && (strchr(";()", character) != NULL);
}

int tokenizer_is_double_possible_special_character(char character){
    return (character != '\0') && (strchr("&|<>!", character) != NULL);
}

int tokenizer_is_white_space(char character){
    return (character != '\0') && (strchr(" \t", character) != NULL);
}

char *tokenizer_error_message(int error) {
    switch (error) {
        case 100:
            return "Unmatched '.";
            break;
        case 101:
            return "Unmatched \".";
            break;
        default:
            return "Unknown error.";
            break;
    }
}