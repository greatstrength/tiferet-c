# Utilities Guide – TifMap

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

`TifMap` is the universal string-keyed container used throughout tiferet-c — for data context maps, step parameters, service parameters, and internal storage in the DI container and registry.

Understanding its **ownership model** is essential for correct memory management: `TifMap` owns its **keys** (heap-copies on insert) but **not its values** (caller-managed `void*`).

## Three Common Patterns

### 1. Data Context Map (String Literals as Values)

The data map passed to `tif_app_ctx_run` typically holds string literals or short-lived stack strings as values. After a successful run, result strings stored by the feature context are heap-allocated and must be freed by the caller.

```c
TifMap data;
tif_map_init(&data);

// String literal values — NOT owned, do not free.
tif_map_set(&data, "a", (void*)"3");
tif_map_set(&data, "b", (void*)"4");

TifResult r = tif_app_ctx_run(&app, "calc.add", &data);

if (TIF_IS_OK(r)) {
    // "result" was stored by the feature context — heap-allocated, caller owns it.
    char* result = (char*)tif_map_get(&data, "result");
    printf("Result: %s\n", result);
    free(result);  // Free the heap-allocated result.
}

// tif_map_destroy frees the keys ("a", "b", "result") but not the values.
tif_map_destroy(&data);
```

### 2. Parameter Map (Heap-Allocated String Values)

Step parameters store heap-allocated strings that are owned by the `TifFeatureEvent`:

```c
TifFeatureEvent step;
tif_feature_event_init(&step, "Sqrt", "exp_event", "result", false, NULL);

// Values are heap-allocated — tif_feature_destroy will free them.
tif_map_set(&step.parameters, "b", tif_strdup("0.5"));

tif_feature_add_step(&feat, step);
// step.parameters is now owned by feat — do NOT destroy step separately.
```

When the feature is destroyed, `tif_feature_event_destroy` iterates the parameters map, but `tif_map_destroy` only frees keys. For the values to be freed, the parameters map must hold tif_strdup'd strings and the caller must iterate and free them — or use `tif_feature_destroy` which takes care of this for `TifFeatureEvent.parameters` if values were set via the standard pattern.

> **Note:** The framework's internal parameter map cleanup frees the keys via `tif_map_destroy`. If you store heap strings as values, free them manually before destroying the map (or before handing ownership to a feature step).

### 3. General Iteration and Cleanup

When a map holds heap-allocated values that need freeing before destruction:

```c
// Free all string values before destroying the map.
TifMapIter iter;
tif_map_iter_init(&iter, &my_map);
const char* key;
void* value;
while (tif_map_iter_next(&iter, &key, &value)) {
    free(value);
}
tif_map_destroy(&my_map);  // Now frees only keys (values already freed).
```

## Basic Operations

```c
TifMap m;
tif_map_init(&m);

// Insert / update
tif_map_set(&m, "key", some_ptr);

// Lookup — returns NULL if not found
void* val = tif_map_get(&m, "key");

// Existence check
bool has = tif_map_has(&m, "key");

// Remove — returns the value pointer (does NOT free it)
void* removed = tif_map_remove(&m, "key");

// Size
size_t n = tif_map_size(&m);

// Destroy — frees all keys, NOT values
tif_map_destroy(&m);
```

## Overwriting an Existing Key

`tif_map_set` replaces the value for an existing key but does **not** free the old value. If the old value is heap-allocated, retrieve and free it first:

```c
char* old = (char*)tif_map_get(&m, "result");
free(old);
tif_map_set(&m, "result", tif_strdup("42"));
```

## Copying a Map

There is no built-in map copy. To copy, iterate and re-insert:

```c
TifMap copy;
tif_map_init(&copy);

TifMapIter iter;
tif_map_iter_init(&iter, &source);
const char* key; void* value;
while (tif_map_iter_next(&iter, &key, &value)) {
    tif_map_set(&copy, key, tif_strdup((const char*)value));
}
```

## Ownership Summary

| Scenario | Key freed by | Value freed by |
|---|---|---|
| `tif_map_destroy` | `TifMap` | Caller (before destroy) |
| `tif_map_remove` | `TifMap` (frees key) | Caller (return value) |
| `tif_map_clear` | `TifMap` | Caller (before clear) |
| String literal values | — | Never (not heap) |
| `tif_strdup` values | — | Caller via `free` |
| Service pointer values | — | Caller via `svc->destroy` |

## Related Documentation

- [`docs/core/utils.md`](../../core/utils.md) — full TifMap API reference and internals
- [`docs/guides/domain/feature.md`](../domain/feature.md) — step parameters pattern
- [`docs/guides/contexts.md`](../contexts.md) — data context usage in the pipeline
