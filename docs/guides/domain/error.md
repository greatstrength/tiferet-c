# Domain Guide – Error

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

The Error domain defines the human-readable error vocabulary of your application. Each `TifError` pairs a machine-readable `error_code` (e.g., `"DIVISION_BY_ZERO"`) with one or more localized message strings, enabling structured, multilingual error responses.

Errors are used in two places:

1. **Raising errors** — domain event implementations call `TIF_VERIFY` or `TIF_RAISE` with an error code constant. The `TifResult` carries the code back to the caller.
2. **Formatting errors** — `TifErrorContext` looks up the error by code and formats its message for the caller's language.

## Creating a `TifError`

```c
#include <tiferet/tiferet.h>

TifError err;

// error_code is derived automatically from id ("division_by_zero" → "DIVISION_BY_ZERO").
TifResult r = tif_error_init(&err,
    "division_by_zero",     // id
    "Division By Zero",     // name
    NULL,                   // description (optional)
    NULL);                  // error_code (NULL = derive from id)

if (TIF_IS_ERR(r)) { /* handle */ }
```

After `tif_error_init`, `err.error_code` is `"DIVISION_BY_ZERO"` (automatically derived).

## Adding Localized Messages

```c
// Add English message.
TIF_PROPAGATE(tif_error_add_message(&err, "en_US", "Cannot divide by zero"));

// Add Spanish message.
TIF_PROPAGATE(tif_error_add_message(&err, "es_ES", "No se puede dividir por cero"));
```

Messages are heap-allocated and owned by the `TifError`. `tif_error_destroy` frees them.

## Retrieving a Message

```c
// Returns a pointer into the error's owned message — do NOT free it.
const char* text = tif_error_format_message(&err, "en_US");
if (text) {
    printf("Error: %s\n", text);
}
```

Returns `NULL` if no message exists for the requested language.

## Destroying a `TifError`

```c
tif_error_destroy(&err);
// All owned strings and the messages array are freed.
// The struct is zeroed — safe to re-init or discard.
```

## Providing a Custom Error Code

If your error code doesn't follow the `tif_str_to_upper_code` derivation rule (dots/underscores → underscores, lowercase → uppercase), pass it explicitly:

```c
TifResult r = tif_error_init(&err, "http_404", "Not Found", NULL, "HTTP_NOT_FOUND");
// err.error_code == "HTTP_NOT_FOUND" (not "HTTP_404")
```

## Using `TifErrorService`

At runtime, the `TifErrorContext` wraps a `TifErrorService` for lookups. Your concrete `TifErrorService` implementation stores errors and serves them by `id` or `error_code`:

```c
// Look up by error_code
const TifError* err = tif_error_ctx_get(&app.error_ctx, "DIVISION_BY_ZERO");

// Or by YAML id
const TifError* err = tif_error_ctx_get(&app.error_ctx, "division_by_zero");

// Format a message into a caller-supplied buffer
char msg[256];
int n = tif_error_ctx_format(&app.error_ctx,
                              r.error_code,
                              "en_US",
                              msg, sizeof(msg));
if (n > 0) printf("Error: %s\n", msg);
```

See [`docs/guides/interfaces.md`](../interfaces.md) for how to implement a `TifErrorService`.

## Error Handling Pattern in Application Code

The typical pattern after calling `tif_app_ctx_run`:

```c
TifResult r = tif_app_ctx_run(&app, "calc.divide", &data);

if (TIF_IS_ERR(r)) {
    char msg[256] = {0};
    int n = tif_error_ctx_format(tif_app_ctx_error_ctx(&app),
                                  r.error_code, "en_US",
                                  msg, sizeof(msg));
    // Fall back to inline message if error not in service.
    if (n <= 0 && r.message) {
        snprintf(msg, sizeof(msg), "%s", r.message);
    }
    printf("Error: %s\n", msg);
}
```

## Defining Error Constants

Register your error codes as `#define` constants alongside the framework's:

```c
// my_app/errors.h
#define MY_ERR_INSUFFICIENT_FUNDS  "INSUFFICIENT_FUNDS"
#define MY_ERR_ACCOUNT_NOT_FOUND   "ACCOUNT_NOT_FOUND"
```

Then use them in event implementations:

```c
TIF_VERIFY(balance >= amount,
           MY_ERR_INSUFFICIENT_FUNDS,
           "Account balance is insufficient");
```

## YAML Configuration Equivalent

When YAML repositories are available, the error above corresponds to:

```yaml
errors:
  division_by_zero:
    name: Division By Zero
    message:
      - lang: en_US
        text: 'Cannot divide by zero'
      - lang: es_ES
        text: 'No se puede dividir por cero'
```

## Related Documentation

- [`docs/core/domain.md`](../../core/domain.md) — domain object conventions
- [`docs/core/events.md`](../../core/events.md) — `TIF_VERIFY`, `TIF_RAISE`, error codes
- [`docs/guides/interfaces.md`](../interfaces.md) — implementing `TifErrorService`
- [`docs/guides/contexts.md`](../contexts.md) — full pipeline wiring
