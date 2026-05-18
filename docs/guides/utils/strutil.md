# Utilities Guide – strutil

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

`strutil` provides the string primitives used throughout tiferet-c: safe duplication, case conversion, key derivation, and emptiness checks. All functions that allocate a new string return a **heap-allocated `char*` that the caller must `free`**.

## `tif_strdup` — Safe String Duplication

The foundation of ownership in tiferet-c. Used in every `_init` function to copy string inputs:

```c
char* copy = tif_strdup("hello");
// ... use copy ...
free(copy);

// Safe with NULL — returns NULL, does not crash.
char* null_copy = tif_strdup(NULL);  // → NULL
```

Never store a raw input pointer in a domain struct. Always `tif_strdup` it:

```c
// Wrong — caller's string could be freed or go out of scope.
err.name = name;

// Correct — struct owns its own copy.
err.name = tif_strdup(name);
```

## `tif_str_make_key` — Registry Key Construction

Builds a `"module_path::class_name"` registry key:

```c
char* key = tif_str_make_key("calc.events", "AddEvent");
// key == "calc.events::AddEvent"

tif_registry_register(&registry, key, add_event_factory);
free(key);  // Free after use.
```

Also used internally by `TifDIContainer` to resolve configurations.

## `tif_str_to_snake` — Display Name to snake_case

Derives a `feature_key` from a feature's display name:

```c
char* key = tif_str_to_snake("Add Number");   // → "add_number"
char* key2 = tif_str_to_snake("HTTP Client"); // → "http_client"
free(key);
free(key2);
```

Used by `tif_feature_init` when `feature_key` is `NULL`.

## `tif_str_to_upper_code` — Identifier to UPPER_CASE Error Code

Derives an error code from an id:

```c
char* code = tif_str_to_upper_code("division_by_zero"); // → "DIVISION_BY_ZERO"
char* code2 = tif_str_to_upper_code("invalid.input");   // → "INVALID_INPUT"
free(code);
free(code2);
```

Dots and underscores become underscores; all letters become uppercase.
Used by `tif_error_init` when `error_code` is `NULL`.

## `tif_str_is_empty` — Null / Empty Check

Returns `true` if the string is `NULL`, zero-length, or whitespace-only:

```c
tif_str_is_empty(NULL)   // true
tif_str_is_empty("")     // true
tif_str_is_empty("   ")  // true
tif_str_is_empty("x")    // false
```

Used universally in `_validate` functions and DI resolution:

```c
TIF_VERIFY(!tif_str_is_empty(cfg->module_path),
           TIF_ERR_SERVICE_RESOLUTION_FAILED,
           cfg->id);
```

## `tif_str_to_upper` and `tif_str_to_lower` — In-Place Conversion

Modify the string in place and return the same pointer:

```c
char* s = tif_strdup("Hello World");
tif_str_to_upper(s);  // s is now "HELLO WORLD"
tif_str_to_lower(s);  // s is now "hello world"
free(s);
```

Use only on heap-allocated strings (never on string literals — undefined behaviour).

## Common Patterns

### Deriving a composite identifier

```c
// Manually derive group_id, feature_key, id from a name:
char* feature_key = tif_str_to_snake("Add Number");  // "add_number"
// ... build id as "calc.add_number" ...
free(feature_key);
```

### Building a registry key from a config

```c
char* key = tif_str_make_key(cfg->module_path, cfg->class_name);
TifResult r = tif_registry_create(&registry, key, NULL, &svc);
free(key);
```

### Validation guard

```c
TIF_VERIFY(!tif_str_is_empty(name), TIF_ERR_COMMAND_PARAMETER_REQUIRED, "name");
```

## Related Documentation

- [`docs/core/utils.md`](../../core/utils.md) — full strutil API reference
- [`docs/core/domain.md`](../../core/domain.md) — ownership rules and `_init` patterns
- [`docs/core/events.md`](../../core/events.md) — using `TIF_VERIFY` with `tif_str_is_empty`
