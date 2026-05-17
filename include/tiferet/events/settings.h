// *** tiferet/events/settings.h
// Domain event infrastructure for the Tiferet C framework.
// Provides verify/raise macros and a callable raise_error function.
//
// In C, domain events are not classes — they are plain functions
// that return TifResult. This header provides the support macros
// that mirror the verify/raise_error pattern across all four languages:
//   Python: self.verify(expr, error_code, message)
//   C#:     Verify(expr, errorCode, message)
//   C++:    DomainEvent::verify(expr, error_code, message)
//   C:      TIF_VERIFY(expr, error_code, message)

#ifndef TIFERET_EVENTS_SETTINGS_H
#define TIFERET_EVENTS_SETTINGS_H

#include <tiferet/assets/constants.h>
#include <tiferet/assets/errors.h>

// *** macros

// ** macro: TIF_VERIFY
/// Verify an expression; return an error result if false.
/// Use inside any function that returns TifResult.
///
/// Usage:
///   TIF_VERIFY(value > 0, TIF_ERR_INVALID_INPUT, "Value must be positive");
#define TIF_VERIFY(expr, err_code, msg)                    \
    do {                                                    \
        if (!(expr)) return TIF_ERR_MSG(err_code, msg);     \
    } while (0)

// ** macro: TIF_VERIFY_CODE
/// Verify an expression; return an error result (code only) if false.
/// Use when no human-readable message is needed.
#define TIF_VERIFY_CODE(expr, err_code)                    \
    do {                                                    \
        if (!(expr)) return TIF_ERR(err_code);              \
    } while (0)

// ** macro: TIF_RAISE
/// Unconditionally return an error result with code and message.
/// Use inside any function that returns TifResult.
#define TIF_RAISE(err_code, msg) return TIF_ERR_MSG(err_code, msg)

// ** macro: TIF_RAISE_CODE
/// Unconditionally return an error result (code only).
#define TIF_RAISE_CODE(err_code) return TIF_ERR(err_code)

// ** macro: TIF_PROPAGATE
/// Propagate an error from a sub-call. If the result is an error, return it.
/// Use to chain fallible calls without explicit if-checks.
///
/// Usage:
///   TIF_PROPAGATE(tif_error_init(&err, "id", "name", NULL, NULL));
#define TIF_PROPAGATE(call)                                \
    do {                                                    \
        TifResult _r = (call);                              \
        if (TIF_IS_ERR(_r)) return _r;                      \
    } while (0)

// *** functions

/// Raise a structured error as a TifResult.
/// This is a callable function (not a macro) for use through
/// function pointers or when a macro is not appropriate.
/// Mirrors the static DomainEvent::raise_error() across all languages.
TifResult tif_raise_error(const char* error_code, const char* message);

#endif // TIFERET_EVENTS_SETTINGS_H
