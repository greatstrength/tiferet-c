// *** tiferet/interfaces/error.h
// Service interface for error retrieval.
// Mirrors Python ErrorService, C# IErrorService,
// and C++ ErrorService.

#ifndef TIFERET_INTERFACES_ERROR_H
#define TIFERET_INTERFACES_ERROR_H

#include <tiferet/interfaces/settings.h>
#include <tiferet/domain/error.h>

#include <stdbool.h>

// *** interfaces

// ** interface: tif_error_service
/// Abstract service contract for retrieving Error definitions.
/// Consumed by ErrorContext to format error messages from
/// persistent configuration (e.g., YAML).
typedef struct TifErrorService {

    /// Base service vtable. Must be the first member.
    TifService base;

    /// Retrieve an error by its identifier or error code.
    /// Accepts either the YAML key (e.g., "invalid_input") or the
    /// derived uppercase error code (e.g., "INVALID_INPUT").
    /// Returns a pointer to the error, or NULL if not found.
    /// The returned pointer is owned by the service — do not free it.
    const TifError* (*get)(struct TifErrorService* self,
                           const char* id_or_code);

    /// Check whether an error exists by its identifier or error code.
    bool (*exists)(struct TifErrorService* self, const char* id_or_code);

} TifErrorService;

#endif // TIFERET_INTERFACES_ERROR_H
