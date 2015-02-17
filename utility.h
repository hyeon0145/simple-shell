#ifndef smsh_utility_h
#define smsh_utility_h

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define SIZE_OF_BUFFER 1024

char *get_prompt(char *buffer, size_t size);

char *get_hostname(char *buffer, size_t size);
char *get_truncated_working_directory(char *buffer, size_t size);
char *get_login_name(char *buffer, size_t size);

char *str_replace(char *buffer, char *subject, char *search, char *replace);

int file_exists(char *filename);

char *duplicate_string(char *text);
#endif
