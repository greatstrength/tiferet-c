// *** tiferet/domain/error.h
// Error domain models for the Tiferet C framework.
// TifErrorMessage carries a localized message text.
// TifError carries an error definition with code derivation
// and multilingual messages.
// Mirrors C++ tiferet/domain/error.h, C# ErrorConfiguration,
// and Python tiferet/domain/error.py.

#ifndef TIFERET_DOMAIN_ERROR_H
#define TIFERET_DOMAIN_ERROR_H

#include <tiferet/assets/errors.h>

#include <stdbool.h>
#include <stddef.h>

// *** models

// ** model: tif_error_message
/// A localized error message.
typedef struct {
    char* lang;     // Owned. Language code (e.g., "en_US").
    char* text;     // Owned. Message text, may contain format placeholders.
} TifErrorMessage;

// ** model: tif_error
/// An error definition with identifier, name, derived error code,
/// and a list of localized messages.
typedef struct {
    char* id;                   // Owned. Unique identifier (e.g., "invalid_input").
    char* name;                 // Owned. Display name.
    char* description;          // Owned. Optional (may be NULL).
    char* error_code;           // Owned. Uppercase code derived from id.
    TifErrorMessage* messages;  // Owned array of messages.
    size_t message_count;       // Number of messages in the array.
} TifError;

// *** functions: TifErrorMessage

/// Initialize an error message. Copies lang and text.
TifResult tif_error_message_init(TifErrorMessage* msg,
                                 const char* lang,
                                 const char* text);

/// Destroy an error message, freeing owned strings.
void tif_error_message_destroy(TifErrorMessage* msg);

/// Validate an error message. Both lang and text must be non-empty.
bool tif_error_message_validate(const TifErrorMessage* msg);

// *** functions: TifError

/// Initialize an error. Copies all string inputs.
/// If error_code is NULL, it is derived from id (uppercase).
/// Messages are not set here — use tif_error_add_message after init.
TifResult tif_error_init(TifError* err,
                         const char* id,
                         const char* name,
                         const char* description,
                         const char* error_code);

/// Destroy an error, freeing all owned strings and messages.
void tif_error_destroy(TifError* err);

/// Validate an error. id and name must be non-empty.
bool tif_error_validate(const TifError* err);

/// Add a localized message to the error. Copies lang and text.
/// Grows the internal messages array.
TifResult tif_error_add_message(TifError* err,
                                const char* lang,
                                const char* text);

/// Find the message text for the given language.
/// Returns the text string, or NULL if no matching language is found.
const char* tif_error_format_message(const TifError* err,
                                     const char* lang);

/// Derive an uppercase error code from an identifier string.
/// Spaces become underscores, letters become uppercase.
/// Returns a heap-allocated string. Caller owns the result.
char* tif_error_derive_error_code(const char* id);

#endif // TIFERET_DOMAIN_ERROR_H
