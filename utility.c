#include "utility.h"

char *get_prompt(char *buffer, size_t size) {
    char inner_buffer[SIZE_OF_BUFFER];
    
    strcpy(buffer, "[");
    get_hostname(inner_buffer, SIZE_OF_BUFFER - 1);
    strcat(buffer, inner_buffer);
    
    strcat(buffer, ":");
    get_truncated_working_directory(inner_buffer, SIZE_OF_BUFFER - 1);
    strcat(buffer, inner_buffer);

    strcat(buffer, "] ");
    get_login_name(inner_buffer, SIZE_OF_BUFFER - 1);
    strcat(buffer, inner_buffer);
    
    strcat(buffer, "% ");
    
    return buffer;
}

char *get_hostname(char *buffer, size_t size) {
    if (gethostname(buffer, size) < 0) {
        return NULL;
    }
    
    char *position = strchr(buffer, '.');
    if (position != NULL) {
        while (*position != '\0') {
            *position = '\0';
            position++;
        }
    }
    
    return buffer;
}

char *get_truncated_working_directory(char *buffer, size_t size) {
    if (getcwd(buffer, size) == NULL) {
        return NULL;
    }
    
    char *position = strrchr(buffer, '/');
    if (position != NULL) {
        size_t length_of_position = strlen(buffer);
        
        position += 1; // beginning '/'
        
        memcpy(buffer, position, length_of_position);
        buffer[length_of_position] = '\0';
    }
    
    return buffer;
}

char *get_login_name(char *buffer, size_t size) {
    getlogin_r(buffer, size);
    
    return buffer;
}

char *str_replace(char *buffer, char *subject, char *search, char *replace)
{
    size_t length_of_subject = strlen(subject);
    size_t length_of_search = strlen(search);
    size_t length_of_replace = strlen(replace);
    
    int difference = (int)(length_of_replace - length_of_search);
    
    int occurrence = 0;
    char *position = strstr(subject, search);
    if (position == NULL) return strdup(subject);
    
    while (position != NULL) {
        position = strstr(position + length_of_search, search);
        occurrence++;
    }
    
    int new_length = (int)(length_of_subject + (occurrence * difference));
    memset(buffer, 0, new_length + 1);
    
    position = strstr(subject, search);
    while (position != NULL) {
        strncat(buffer, subject, position - subject);
        strcat(buffer, replace);
        
        subject = position + length_of_search;
        position = strstr(subject, search);
    }
    strcat(buffer, subject);
    
    return buffer;
}

int file_exists(char *filename) {
    struct stat status;
    
    return !stat(filename, &status);
}

char *duplicate_string(char *text)
{
    size_t length = strlen(text + 1);
    char *duplicated_text = malloc(length);
    strcpy(duplicated_text, text);

    return duplicated_text;
}
