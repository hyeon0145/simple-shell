#ifndef smsh_parser_h
#define smsh_parser_h

#include <string.h>
#include <assert.h>

#include "utility.h"
#include "list.h"
#include "tokenizer.h"

enum PARSER_COMMAND {
    PARSER_COMMAND_NONE,
    PARSER_COMMAND_NORMAL,
    PARSER_COMMAND_AND,
    PARSER_COMMAND_OR,
    PARSER_COMMAND_PIPE,
    PARSER_COMMAND_BACKGROUND,
};

enum PARSER_REDIRECTION {
    PARSER_REDIRECTION_NONE,
    PARSER_REDIRECTION_STANDARD_IN, /* < */
    PARSER_REDIRECTION_TRUNCATING_STANDARD_OUT, /* > */
    PARSER_REDIRECTION_APPENDING_STANDARD_OUT, /* >> */
    PARSER_REDIRECTION_OVERWRITING_STANDARD_OUT, /* >! */
};

struct parser_redirection {
    enum PARSER_REDIRECTION type;
    char *filename;
};

struct parser_command {
    enum PARSER_COMMAND type;

    char *filename;
    struct list *parameters; /* char * */

    struct parser_redirection *in_redirection;
    struct parser_redirection *out_redirection;
};

struct parser_data {
    struct list *tokens;
    struct list_iterator *iterator;

    struct parser_command *command;
    
    int error;
};

struct list *parser_parse(struct list *tokens, int *error);
void parser_free(struct list *sequence_of_commands_list);

struct parser_command *parser_create_command();
void parser_free_command(struct parser_command *command);

struct parser_redirection *parser_create_redirection();
void parser_free_redirection(struct parser_redirection *redirection);

void parser_handle_command(struct parser_data *data);
void parser_handle_filename(struct parser_data *data);
void parser_handle_parameter_and_redirector(struct parser_data *data);
void parser_handle_parameter(struct parser_data *data);
void parser_handle_redirector(struct parser_data *data);
void parser_handle_separator(struct parser_data *data);

struct list *parser_get_sequence_of_commands(struct list_iterator *iterator);

char *parser_stringify_command(struct parser_command *command);

int parser_is_redirector_token(struct tokenizer_token *token);
int parser_is_separator_token(struct tokenizer_token *token);

char *parser_error_message(int error);

#define parser_is_word(token) \
    (((token)->type == TOKENIZER_TOKEN_SIMPLE_WORD) || ((token)->type == TOKENIZER_TOKEN_QUOTED_WORD))

#define parser_is_parameter_token(token) parser_is_word(token)

#endif