# Utilities in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

The utilities layer provides the foundational infrastructure used throughout tiferet-c. Three utility modules underpin every layer of the framework:

| Module | Purpose |
|---|---|
| `TifMap` | String-keyed hash map — the primary data container |
| `TifVector` | Dynamic array of `void*` |
| `strutil` | String duplication, transformation, and key generation |

All utilities are pure C11 with no external dependencies.

## `TifMap` — String-Keyed Hash Map

Defined in `include/tiferet/utils/map.h`.

`TifMap` is an open-addressing hash map with linear probing and tombstone removal. Keys are owned `char*` strings (heap-copied on insert, freed on remove/destroy). Values are caller-managed `void*`.

### Key Properties

- **Key ownership:** `TifMap` copies keys on `tif_map_set`; frees them on `tif_map_remove` and `tif_map_destroy`.
- **Value ownership:** The caller manages value lifetimes. `tif_map_destroy` does **not** free values.
- **Hash function:** FNV-1a for good distribution over string keys.
- **Load factor:** Resizes at 75% capacity, doubling each time.

### API

```c
void   tif_map_init(TifMap* map);
void   tif_map_destroy(TifMap* map);     // Frees keys, not values.
bool   tif_map_set(TifMap* map, const char* key, void* value);
void*  tif_map_get(const TifMap* map, const char* key);
bool   tif_map_has(const TifMap* map, const char* key);
void*  tif_map_remove(TifMap* map, const char* key);  // Returns removed value.
size_t tif_map_size(const TifMap* map);
bool   tif_map_is_empty(const TifMap* map);
void   tif_map_clear(TifMap* map);       // Frees keys, not values.
```

### Iteration

```c
TifMapIter iter;
tif_map_iter_init(&iter, &map);
const char* key;
void*       value;
while (tif_map_iter_next(&iter, &key, &value)) {
    printf("%s → %s\n", key, (const char*)value);
}
```

### Usage Patterns

**String-to-string parameter map** (as used in `TifFeatureEvent.parameters`):

```c
TifMap params;
tif_map_init(&params);
tif_map_set(&params, "b", tif_strdup("0.5"));  // Value is heap-allocated.

const char* b = (const char*)tif_map_get(&params, "b");

// Cleanup: free values before destroying the map.
TifMapIter iter;
tif_map_iter_init(&iter, &params);
const char* k; void* v;
while (tif_map_iter_next(&iter, &k, &v)) free(v);
tif_map_destroy(&params);
```

**Service map** (as used in `TifDIContainer.services`):

```c
// Values are TifService* — destroyed manually before tif_map_destroy.
TifService* svc = /* ... */;
tif_map_set(&services, "add_event", svc);

// On teardown:
while (tif_map_iter_next(&iter, &k, &v)) {
    TifService* s = (TifService*)v;
    if (s && s->destroy) s->destroy(s);
}
tif_map_destroy(&services);
```

### Important: Value Ownership

`TifMap` is intentionally neutral about value lifetimes. The pattern throughout tiferet-c is:

- If values are heap-allocated strings → iterate and `free` each value before `tif_map_destroy`.
- If values are services → iterate and call `->destroy` before `tif_map_destroy`.
- If values are string literals or stack pointers → no cleanup needed; `tif_map_destroy` only frees keys.

## `TifVector` — Dynamic Array

Defined in `include/tiferet/utils/vector.h`.

`TifVector` is a `realloc`-grown array of `void*`. Values are caller-managed.

### API

```c
void   tif_vector_init(TifVector* vec);
void   tif_vector_destroy(TifVector* vec);   // Frees internal array, not values.
bool   tif_vector_push(TifVector* vec, void* item);
void*  tif_vector_get(const TifVector* vec, size_t index);
bool   tif_vector_remove(TifVector* vec, size_t index);
void   tif_vector_clear(TifVector* vec);
size_t tif_vector_size(const TifVector* vec);
bool   tif_vector_is_empty(const TifVector* vec);
```

### Usage

```c
TifVector vec;
tif_vector_init(&vec);

tif_vector_push(&vec, some_ptr);
void* item = tif_vector_get(&vec, 0);

tif_vector_destroy(&vec);  // Only frees the internal array.
```

## `strutil` — String Utilities

Defined in `include/tiferet/utils/strutil.h`.

All functions that return `char*` return a **heap-allocated** string that the caller owns and must `free`.

### `tif_strdup`

Duplicates a string. Returns `NULL` if `src` is `NULL`.

```c
char* copy = tif_strdup("hello");
// ...
free(copy);
```

Used universally in `_init` functions to copy string inputs.

### `tif_str_to_snake`

Converts a display name to `snake_case` — used to derive `feature_key` from `name`:

```c
char* key = tif_str_to_snake("Add Number");  // → "add_number"
free(key);
```

### `tif_str_to_upper_code`

Converts an identifier to `UPPER_CASE` — used to derive `error_code` from `id`:

```c
char* code = tif_str_to_upper_code("division_by_zero");  // → "DIVISION_BY_ZERO"
free(code);
```

Dots and underscores become underscores; letters become uppercase.

### `tif_str_make_key`

Builds a registry key from `module_path` and `class_name`:

```c
char* key = tif_str_make_key("calc.events", "AddEvent");
// → "calc.events::AddEvent"
tif_registry_register(&registry, key, add_event_factory);
free(key);
```

### `tif_str_is_empty`

Returns `true` if the string is `NULL` or empty after whitespace trimming. Used in `_validate` functions:

```c
if (tif_str_is_empty(feat->id)) {
    TIF_RAISE(TIF_ERR_COMMAND_PARAMETER_REQUIRED, "id");
}
```

### `tif_str_to_upper` / `tif_str_to_lower`

In-place conversion (modifies the string directly, returns the same pointer):

```c
char* s = tif_strdup("Hello");
tif_str_to_upper(s);  // s is now "HELLO"
free(s);
```

## Package Layout

```
include/tiferet/utils/
├── map.h       — TifMap + TifMapIter + TifMapEntry
├── vector.h    — TifVector
└── strutil.h   — String utility functions

src/utils/
├── map.c
├── vector.c
└── strutil.c
```

## Conclusion

`TifMap`, `TifVector`, and `strutil` provide the minimal infrastructure needed to implement the full Tiferet framework in pure C11. Their explicit ownership model — caller manages values, utilities manage keys/arrays — requires discipline but eliminates hidden allocations and makes lifecycle management predictable.

Explore `include/tiferet/utils/` and `src/utils/` for implementation details.
