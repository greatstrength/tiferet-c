// *** tiferet/assets/errors.h
// Structured error result type for the Tiferet C framework.
// TifResult replaces exceptions — all fallible functions return it.
// Mirrors C++ TiferetError (value type) and Expected<T>.

#ifndef TIFERET_ASSETS_ERRORS_H
#define TIFERET_ASSETS_ERRORS_H

#include <stddef.h>

// *** types

// ** type: tif_result
/// Lightweight result type for error propagation.
/// error_code is NULL on success, non-NULL on failure.
typedef struct {
    const char* error_code;   // NULL on success
    const char* message;      // Optional human-readable message
} TifResult;

// *** macros

// ** macro: TIF_OK
/// Construct a success result.
#define TIF_OK ((TifResult){NULL, NULL})

// ** macro: TIF_ERR
/// Construct an error result with a code only.
#define TIF_ERR(code) ((TifResult){(code), NULL})

// ** macro: TIF_ERR_MSG
/// Construct an error result with a code and message.
#define TIF_ERR_MSG(code, msg) ((TifResult){(code), (msg)})

// ** macro: TIF_IS_OK
/// Test whether a result is successful.
#define TIF_IS_OK(r) ((r).error_code == NULL)

// ** macro: TIF_IS_ERR
/// Test whether a result is an error.
#define TIF_IS_ERR(r) ((r).error_code != NULL)

// *** functions

/// Format a TifResult into a human-readable string.
/// Writes into buf (up to buf_size bytes including NUL).
/// Returns the number of characters written (excluding NUL).
int tif_result_format(const TifResult* result, char* buf, size_t buf_size);

#endif // TIFERET_ASSETS_ERRORS_H
