// *** tiferet/contexts/error.h
// Error formatting context for the Tiferet C framework.
// Provides error lookup and message formatting via TifErrorService.
// Mirrors C++ ErrorContext and Python ErrorContext.

#ifndef TIFERET_CONTEXTS_ERROR_H
#define TIFERET_CONTEXTS_ERROR_H

#include <tiferet/interfaces/error.h>
#include <tiferet/domain/error.h>

#include <stdbool.h>
#include <stddef.h>

// *** contexts

// ** context: tif_error_context
/// Provides error lookup and message formatting using a TifErrorService.
/// Thin wrapper around the error service for use in application code
/// when handling TifResult errors.
/// Mirrors C++ ErrorContext.
typedef struct {
    TifErrorService* error_service;     // Borrowed. Error service for lookups.
} TifErrorContext;

// *** functions

/// Initialize an error context with a reference to the error service.
///
/// :param ctx:           The error context to initialize.
/// :param error_service: Borrowed reference to the error service.
void tif_error_ctx_init(TifErrorContext* ctx,
                        TifErrorService* error_service);

/// Retrieve an error definition by its identifier or error code.
/// Accepts the YAML key (e.g., "invalid_input") or uppercase code
/// (e.g., "INVALID_INPUT"). Returns NULL if not found.
/// The returned pointer is owned by the service — do not free it.
const TifError* tif_error_ctx_get(const TifErrorContext* ctx,
                                  const char* id_or_code);

/// Look up an error and copy its message for the given language into buf.
/// Returns the number of characters written (excluding NUL), or 0 if the
/// error or language is not found.
///
/// :param ctx:        The error context.
/// :param id_or_code: The error identifier or error code.
/// :param lang:       Language code (e.g., "en_US"). NULL defaults to "en_US".
/// :param buf:        Buffer to write the formatted message into.
/// :param buf_size:   Size of buf in bytes.
int tif_error_ctx_format(const TifErrorContext* ctx,
                         const char* id_or_code,
                         const char* lang,
                         char* buf,
                         size_t buf_size);

/// Check whether an error definition exists by identifier or error code.
bool tif_error_ctx_exists(const TifErrorContext* ctx,
                          const char* id_or_code);

#endif // TIFERET_CONTEXTS_ERROR_H
