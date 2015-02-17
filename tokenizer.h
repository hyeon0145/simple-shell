#ifndef smsh_tokenizer_h
#define smsh_tokenizer_h

#include <string.h>
#include <assert.h>

#include "list.h"
#include "utility.h"

enum TOKENIZER_TOKEN {
	TOKENIZER_TOKEN_NONE,
	TOKENIZER_TOKEN_WHITE_SPACE,
	TOKENIZER_TOKEN_SIMPLE_WORD,
	TOKENIZER_TOKEN_QUOTED_WORD,
	TOKENIZER_TOKEN_SINGLE_SPECIAL_CHARACTER,
	TOKENIZER_TOKEN_DOUBLE_SPECIAL_CHARACTERS,
};

struct tokenizer_data {
	char *source;
	char *current_source;

	enum TOKENIZER_TOKEN type;
	char token[1024];
	char *current_token;

	int error;
};

struct tokenizer_token {
	enum TOKENIZER_TOKEN type;
	char *data;
};

struct list *tokenizer_tokenize(char *source, int *error);
void tokenizer_free(struct list *tokens);

struct tokenizer_token *tokenizer_create_token();
void tokenizer_free_token(struct tokenizer_token *token);

void tokenizer_handle_ready(struct tokenizer_data *data);
void tokenizer_handle_white_space(struct tokenizer_data *data);
void tokenizer_handle_simple_word(struct tokenizer_data *data);
void tokenizer_handle_escape_sequence(struct tokenizer_data *data);
void tokenizer_handle_quoted_word(struct tokenizer_data *data);
void tokenizer_handle_single_quoted_word(struct tokenizer_data *data);
void tokenizer_handle_double_quoted_word(struct tokenizer_data *data);
void tokenizer_handle_single_special_character(struct tokenizer_data *data);
void tokenizer_handle_double_possible_special_character(struct tokenizer_data *data);
void tokenizer_handle_double_special_character(struct tokenizer_data *data);

int tokenizer_is_word(char character);
int tokenizer_is_single_special_character(char character);
int tokenizer_is_double_possible_special_character(char character);
int tokenizer_is_white_space(char character);

char *tokenizer_error_message(int error);

#define tokenizer_is_escape_character(character) ((character) == '\\')
#define tokenizer_is_single_quote(character) ((character) == '\'')
#define tokenizer_is_double_quote(character) ((character) == '"')
#define tokenizer_is_quote(character) \
	(tokenizer_is_single_quote((character)) || tokenizer_is_double_quote((character)))
#define tokenizer_is_special_character(character) \
	(tokenizer_is_single_special_character((character)) || tokenizer_is_double_possible_special_character((character)))

#endif






