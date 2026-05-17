// *** src/utils/strutil.c
// Implementation of string utility functions.

#include <tiferet/utils/strutil.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// *** functions

// ** function: tif_strdup
char* tif_strdup(const char* src) {

    // Handle NULL input.
    if (!src) return NULL;

    // Allocate and copy.
    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (!dst) return NULL;

    memcpy(dst, src, len + 1);
    return dst;
}

// ** function: tif_str_to_upper
char* tif_str_to_upper(char* str) {

    // Handle NULL input.
    if (!str) return NULL;

    // Convert each character to uppercase.
    for (char* p = str; *p; ++p) {
        *p = (char)toupper((unsigned char)*p);
    }

    return str;
}

// ** function: tif_str_to_lower
char* tif_str_to_lower(char* str) {

    // Handle NULL input.
    if (!str) return NULL;

    // Convert each character to lowercase.
    for (char* p = str; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }

    return str;
}

// ** function: tif_str_to_snake
char* tif_str_to_snake(const char* name) {

    // Handle NULL input.
    if (!name) return NULL;

    // Allocate output buffer (same length as input).
    size_t len = strlen(name);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    // Convert spaces to underscores, letters to lowercase.
    for (size_t i = 0; i < len; ++i) {
        if (name[i] == ' ') {
            result[i] = '_';
        } else {
            result[i] = (char)tolower((unsigned char)name[i]);
        }
    }

    result[len] = '\0';
    return result;
}

// ** function: tif_str_to_upper_code
char* tif_str_to_upper_code(const char* id) {

    // Handle NULL input.
    if (!id) return NULL;

    // Allocate output buffer (same length as input).
    size_t len = strlen(id);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    // Convert spaces/dots to underscores, letters to uppercase.
    for (size_t i = 0; i < len; ++i) {
        if (id[i] == ' ' || id[i] == '.') {
            result[i] = '_';
        } else {
            result[i] = (char)toupper((unsigned char)id[i]);
        }
    }

    result[len] = '\0';
    return result;
}

// ** function: tif_str_make_key
char* tif_str_make_key(const char* module_path, const char* class_name) {

    // Handle NULL inputs.
    if (!module_path || !class_name) return NULL;

    // Calculate total length: module_path + "::" + class_name + NUL.
    size_t mp_len = strlen(module_path);
    size_t cn_len = strlen(class_name);
    char* result = (char*)malloc(mp_len + 2 + cn_len + 1);
    if (!result) return NULL;

    // Build "module_path::class_name".
    memcpy(result, module_path, mp_len);
    result[mp_len] = ':';
    result[mp_len + 1] = ':';
    memcpy(result + mp_len + 2, class_name, cn_len);
    result[mp_len + 2 + cn_len] = '\0';

    return result;
}

// ** function: tif_str_is_empty
bool tif_str_is_empty(const char* str) {

    // NULL is empty.
    if (!str) return true;

    // Skip whitespace.
    while (*str) {
        if (!isspace((unsigned char)*str)) return false;
        ++str;
    }

    return true;
}
