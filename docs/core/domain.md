# Domain Objects in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

Domain objects are the structural core of the Tiferet C framework. Every domain concept — errors, features, app interfaces, service configurations — is expressed as a plain C struct following the `_init` / `_destroy` / `_validate` convention.

Unlike the Python framework (which uses Pydantic `BaseModel`), C has no base class inheritance. The convention is **agreement-based**: any struct that follows the `_init` / `_destroy` pattern is a domain object.

Domain objects serve a **dual role**:

1. **Runtime domain models** — active participants in application execution, returned by context functions and consumed by the feature pipeline.
2. **Structural foundation for the mappers layer** — aggregates embed the domain struct as `data` and add mutation functions; transfer objects embed it and add serialization functions.

## The Domain Object Convention

Every domain struct has three associated functions:

| Function | Signature | Purpose |
|---|---|---|
| `_init` | `TifResult _init(T* obj, ...)` | Allocate owned fields, derive missing values, initialise maps/arrays |
| `_destroy` | `void _destroy(T* obj)` | Free all owned fields, reset struct to zero |
| `_validate` | `bool _validate(const T* obj)` | Check required fields are non-empty |

### Ownership Rules

- **All `char*` fields are owned** — heap-allocated via `tif_strdup` in `_init`, freed via `free` in `_destroy`.
- **Array fields** (e.g., `steps`, `messages`) are heap-allocated via `realloc` as elements are added. `_destroy` iterates the array and destroys each element before freeing.
- **`TifMap` fields** are initialised with `tif_map_init` and destroyed with `tif_map_destroy`.
- Callers pass `const char*` inputs to `_init`; the function copies them. The caller retains ownership of the inputs.

### Zero-Initialise on Entry

`_init` functions always `memset` to zero first:

```c
memset(obj, 0, sizeof(T));
```

This ensures that `_destroy` is safe to call even if `_init` fails partway through (partial initialisation → partial `free`).

## Domain Modules

### `TifError` and `TifErrorMessage` (`domain/error.h`)

```c
typedef struct {
    char* lang;     // Owned.
    char* text;     // Owned.
} TifErrorMessage;

typedef struct {
    char* id;                   // Owned. e.g. "invalid_input"
    char* name;                 // Owned. Display name.
    char* description;          // Owned. Optional.
    char* error_code;           // Owned. Uppercase code derived from id.
    TifErrorMessage* messages;  // Owned heap array.
    size_t message_count;
} TifError;
```

Key behaviours:
- `tif_error_init` derives `error_code` from `id` via `tif_error_derive_error_code` if not provided.
- `tif_error_add_message` grows the `messages` array with `realloc`.
- `tif_error_format_message(err, lang)` returns the text for the given language (or `NULL`).

### `TifFeature` and `TifFeatureEvent` (`domain/feature.h`)

```c
typedef struct {
    char* name;
    char* service_id;   // Owned. Links to DI container entry.
    TifMap parameters;  // Owned. Step-level params (string→string).
    char* data_key;     // Owned. Key under which the result is stored.
    bool  pass_on_error;
    char* condition;    // Owned. Optional.
} TifFeatureEvent;

typedef struct {
    char* id;               // e.g. "calc.add"
    char* name;
    char* description;
    char* group_id;         // e.g. "calc"
    char* feature_key;      // e.g. "add"
    TifFeatureEvent* steps; // Owned heap array.
    size_t step_count;
} TifFeature;
```

Key behaviours:
- `tif_feature_init` derives `group_id`, `feature_key`, and `id` from whichever inputs are provided (mirrors Python `Feature._derive_keys`).
- `tif_feature_add_step(feat, step)` takes **ownership** of `step` via shallow copy — the caller must **not** destroy the source event after this call.

### `TifServiceConfiguration` and `TifFlaggedDependency` (`domain/di.h`)

```c
typedef struct {
    char*  module_path;
    char*  class_name;
    char*  flag;
    TifMap parameters;
} TifFlaggedDependency;

typedef struct {
    char*                  id;
    char*                  name;
    char*                  module_path;
    char*                  class_name;
    TifMap                 parameters;
    TifFlaggedDependency*  dependencies;
    size_t                 dependency_count;
} TifServiceConfiguration;
```

Used by `TifDIContainer` to resolve services. `tif_service_config_get_dependency` scans the `dependencies` array for the first flag match.

### `TifAppInterface` and `TifAppServiceDependency` (`domain/app.h`)

Application interface definitions consumed by the app-level DI layer.

## Creating a Domain Object

### 1. Define the struct in `include/tiferet/domain/<name>.h`

```c
// ** model: tif_calculator_result
typedef struct {
    char* operation;    // Owned. e.g. "add"
    char* result;       // Owned. Numeric result as string.
} TifCalculatorResult;
```

### 2. Declare the lifecycle functions

```c
TifResult tif_calc_result_init(TifCalculatorResult* r,
                               const char* operation,
                               const char* result);
void tif_calc_result_destroy(TifCalculatorResult* r);
bool tif_calc_result_validate(const TifCalculatorResult* r);
```

### 3. Implement in `src/domain/<name>.c`

```c
TifResult tif_calc_result_init(TifCalculatorResult* r,
                               const char* operation,
                               const char* result) {
    memset(r, 0, sizeof(TifCalculatorResult));

    r->operation = tif_strdup(operation);
    if (operation && !r->operation) return TIF_ERR("ALLOC_FAILED");

    r->result = tif_strdup(result);
    if (result && !r->result) {
        tif_calc_result_destroy(r);
        return TIF_ERR("ALLOC_FAILED");
    }

    return TIF_OK;
}

void tif_calc_result_destroy(TifCalculatorResult* r) {
    free(r->operation);
    free(r->result);
    memset(r, 0, sizeof(TifCalculatorResult));
}

bool tif_calc_result_validate(const TifCalculatorResult* r) {
    return !tif_str_is_empty(r->operation) && !tif_str_is_empty(r->result);
}
```

## Best Practices

- Zero-initialise at the start of every `_init`.
- Call `_destroy` on partial init failure to avoid leaks.
- Never store raw input pointers — always `tif_strdup`.
- `_destroy` must be safe to call on a zero-initialised struct (`free(NULL)` is a no-op).
- Keep domain objects **read-only** after initialisation — mutation belongs in aggregates (`mappers/`).

## Package Layout

```
include/tiferet/domain/
├── settings.h     — documentation-only: describes the convention
├── error.h        — TifErrorMessage, TifError
├── feature.h      — TifFeatureEvent, TifFeature
├── app.h          — TifAppServiceDependency, TifAppInterface
└── di.h           — TifFlaggedDependency, TifServiceConfiguration

src/domain/
├── error.c
├── feature.c
├── app.c
└── di.c
```

## Conclusion

Domain objects in tiferet-c are plain C structs governed by a consistent `_init` / `_destroy` / `_validate` convention. Strict ownership semantics — all `char*` fields owned and heap-allocated — make lifecycle management predictable. The mappers layer builds on these structs without duplicating their field definitions.

Explore `include/tiferet/domain/` and `src/domain/` for implementation details.
