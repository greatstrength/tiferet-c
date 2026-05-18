# Events & Error Propagation in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

C has no exceptions. tiferet-c uses a value-based error propagation model built around `TifResult` — a lightweight struct returned by all fallible functions. A set of macros (`TIF_VERIFY`, `TIF_RAISE`, `TIF_PROPAGATE`) provides the same guard-and-early-return patterns as `verify` / `raise_error` in the Python and C++ frameworks.

## The `TifResult` Type

Defined in `include/tiferet/assets/errors.h`:

```c
typedef struct {
    const char* error_code;   // NULL on success; non-NULL on failure.
    const char* message;      // Optional human-readable message.
} TifResult;
```

The `error_code` field is the source of truth. Use the provided macros — never compare `error_code` directly.

### Core Macros

```c
TIF_OK                         // Construct a success result: {NULL, NULL}
TIF_ERR(code)                  // Construct an error with a code only
TIF_ERR_MSG(code, msg)         // Construct an error with a code and message
TIF_IS_OK(r)                   // True if r.error_code == NULL
TIF_IS_ERR(r)                  // True if r.error_code != NULL
```

### Usage Pattern

```c
TifResult r = tif_feature_init(&feat, "calc.add", "Add", "calc", "add", NULL);
if (TIF_IS_ERR(r)) {
    fprintf(stderr, "Error [%s]: %s\n", r.error_code, r.message ? r.message : "");
    return;
}
```

## Error Propagation Macros

Defined in `include/tiferet/events/settings.h`:

### `TIF_VERIFY`

Early-return if an expression is false. The idiomatic domain rule guard:

```c
// Verify that b is non-zero before dividing.
TIF_VERIFY(b != 0.0, TIF_ERR_DIVISION_BY_ZERO, "Cannot divide by zero");
```

Signature: `TIF_VERIFY(expr, error_code, message)`

### `TIF_VERIFY_CODE`

Same as `TIF_VERIFY` but without a message string:

```c
TIF_VERIFY_CODE(ptr != NULL, TIF_ERR_FEATURE_NOT_FOUND);
```

### `TIF_RAISE`

Unconditional error return — use when the error condition is already known:

```c
if (module_path == NULL || class_name == NULL) {
    TIF_RAISE(TIF_ERR_SERVICE_RESOLUTION_FAILED, config->id);
}
```

### `TIF_RAISE_CODE`

Same as `TIF_RAISE` but without a message:

```c
TIF_RAISE_CODE(TIF_ERR_INVALID_MODEL_ATTRIBUTE);
```

### `TIF_PROPAGATE`

Chains fallible sub-calls. If the sub-call returns an error, that error is immediately returned from the current function:

```c
TIF_PROPAGATE(tif_error_init(&err, id, name, NULL, NULL));
TIF_PROPAGATE(tif_error_add_message(&err, "en_US", "Cannot divide by zero"));
```

Equivalent to:
```c
TifResult _r = tif_error_init(&err, id, name, NULL, NULL);
if (TIF_IS_ERR(_r)) return _r;
```

## Error Code Constants

All error codes are `#define` string constants in `include/tiferet/assets/constants.h`:

```c
// Parameter validation
#define TIF_ERR_COMMAND_PARAMETER_REQUIRED  "COMMAND_PARAMETER_REQUIRED"

// Feature errors
#define TIF_ERR_FEATURE_NOT_FOUND           "FEATURE_NOT_FOUND"
#define TIF_ERR_FEATURE_COMMAND_LOADING_FAILED "FEATURE_COMMAND_LOADING_FAILED"
#define TIF_ERR_FEATURE_STEP_EXECUTION_FAILED  "FEATURE_STEP_EXECUTION_FAILED"

// DI errors
#define TIF_ERR_SERVICE_NOT_REGISTERED      "SERVICE_NOT_REGISTERED"
#define TIF_ERR_SERVICE_RESOLUTION_FAILED   "SERVICE_RESOLUTION_FAILED"

// Application errors
#define TIF_ERR_DIVISION_BY_ZERO            "DIVISION_BY_ZERO"
#define TIF_ERR_INVALID_INPUT               "INVALID_INPUT"
```

Always use the `#define` constant — never a raw string literal — so callers can compare with `==` or match against `TifError.error_code`.

## `tif_raise_error`

A callable function version of `TIF_RAISE` for use through function pointers or when a macro is inconvenient:

```c
TifResult tif_raise_error(const char* error_code, const char* message);
```

## Comparison: `TIF_VERIFY` vs `TIF_RAISE` vs `TIF_PROPAGATE`

| Macro | When to use |
|---|---|
| `TIF_VERIFY(expr, code, msg)` | Domain rule guard — condition that must be true |
| `TIF_RAISE(code, msg)` | Unconditional error — condition already evaluated |
| `TIF_PROPAGATE(call)` | Chain fallible sub-calls without explicit `if` |

## Domain Event Implementations

In tiferet-c, domain events are implemented as `TifExecutableEvent` structs — vtable-based services with an `execute` function pointer. They use these macros internally:

```c
static TifResult divide_event_execute(TifExecutableEvent* self,
                                      const TifMap* data,
                                      const TifMap* params,
                                      char** result_out) {
    (void)self;
    double a = get_value(params, data, "a");
    double b = get_value(params, data, "b");

    // Guard: fail fast if b is zero.
    TIF_VERIFY(b != 0.0, TIF_ERR_DIVISION_BY_ZERO, "Cannot divide by zero");

    *result_out = format_result(a / b);
    return TIF_OK;
}
```

See [`docs/core/contexts.md`](contexts.md) for details on `TifExecutableEvent`.

## `tif_result_format`

Formats a `TifResult` into a human-readable string:

```c
int tif_result_format(const TifResult* result, char* buf, size_t buf_size);
```

Useful for logging or error reporting:

```c
char buf[256];
tif_result_format(&r, buf, sizeof(buf));
fprintf(stderr, "%s\n", buf);
```

## Best Practices

- Every fallible function returns `TifResult`.
- Check `TIF_IS_ERR(r)` immediately after any call that can fail.
- Use `TIF_PROPAGATE` to chain sub-calls without boilerplate.
- Use `TIF_VERIFY` for domain rule checks inside event execute functions.
- Use `#define` error code constants — never inline strings.
- On partial initialisation failure, call `_destroy` before returning the error.

## Package Layout

```
include/tiferet/assets/
├── errors.h     — TifResult type and TIF_OK / TIF_ERR / TIF_IS_OK macros
└── constants.h  — All TIF_ERR_* error code string constants

include/tiferet/events/
└── settings.h   — TIF_VERIFY, TIF_RAISE, TIF_PROPAGATE macros; tif_raise_error()

src/events/
└── settings.c   — tif_raise_error() implementation
```
