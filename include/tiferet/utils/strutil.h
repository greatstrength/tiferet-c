// *** tiferet/utils/strutil.h
// String utility functions for the Tiferet C framework.
// All returned char* pointers are heap-allocated and caller-owned.

#ifndef TIFERET_UTILS_STRUTIL_H
#define TIFERET_UTILS_STRUTIL_H

#include <stdbool.h>
#include <stddef.h>

// *** functions

/// Duplicate a string. Returns a heap-allocated copy.
/// Returns NULL if src is NULL.
char* tif_strdup(const char* src);

/// Convert a string to uppercase in place.
/// Returns str for chaining.
char* tif_str_to_upper(char* str);

/// Convert a string to lowercase in place.
/// Returns str for chaining.
char* tif_str_to_lower(char* str);

/// Derive a snake_case key from a display name.
/// Spaces become underscores, letters become lowercase.
/// Returns a heap-allocated string. Caller owns the result.
char* tif_str_to_snake(const char* name);

/// Derive an UPPER_CASE error code from an identifier.
/// Spaces and dots become underscores, letters become uppercase.
/// Returns a heap-allocated string. Caller owns the result.
char* tif_str_to_upper_code(const char* id);

/// Build a registry key from module_path and class_name.
/// Format: "module_path::class_name".
/// Returns a heap-allocated string. Caller owns the result.
char* tif_str_make_key(const char* module_path, const char* class_name);

/// Check if a string is NULL or empty after trimming whitespace.
bool tif_str_is_empty(const char* str);

#endif // TIFERET_UTILS_STRUTIL_H
