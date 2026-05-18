# Interfaces Guide – Implementing Custom Services

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

`TifFeatureService` and `TifErrorService` are the two services consumed directly by `TifAppContext`. You must provide concrete implementations of both to run the feature pipeline.

This guide shows how to implement in-memory versions — suitable for programmatic wiring (as in `examples/calculator/main.c`) and as a reference pattern for future YAML-backed repository implementations.

## Implementing `TifFeatureService`

### Struct Layout

```c
// Embed the interface as the first member.
typedef struct {
    TifFeatureService vtable;   // Must be first.
    TifFeature*       features; // Borrowed array of features.
    size_t            count;
} MyFeatureService;
```

### Function Implementations

```c
static const TifFeature* my_feature_get(TifFeatureService* self,
                                        const char* id) {
    MyFeatureService* svc = (MyFeatureService*)self;
    for (size_t i = 0; i < svc->count; ++i) {
        if (svc->features[i].id &&
            strcmp(svc->features[i].id, id) == 0) {
            return &svc->features[i];
        }
    }
    return NULL;
}

static bool my_feature_exists(TifFeatureService* self, const char* id) {
    return my_feature_get(self, id) != NULL;
}

static void my_feature_destroy(TifService* self) {
    (void)self;  // Stack-allocated — nothing to free.
}
```

### Wiring

```c
MyFeatureService feature_svc;
feature_svc.vtable.base.destroy = my_feature_destroy;
feature_svc.vtable.get          = my_feature_get;
feature_svc.vtable.exists       = my_feature_exists;
feature_svc.features            = features_array;
feature_svc.count               = features_count;

// Pass as TifFeatureService* to tif_app_ctx_init.
tif_app_ctx_init(&app, (TifFeatureService*)&feature_svc, ...);
```

## Implementing `TifErrorService`

### Struct Layout

```c
typedef struct {
    TifErrorService vtable;  // Must be first.
    TifError*       errors;
    size_t          count;
} MyErrorService;
```

### Function Implementations

The `get` function accepts either the YAML `id` (e.g., `"division_by_zero"`) or the derived `error_code` (e.g., `"DIVISION_BY_ZERO"`):

```c
static const TifError* my_error_get(TifErrorService* self,
                                    const char* id_or_code) {
    MyErrorService* svc = (MyErrorService*)self;
    for (size_t i = 0; i < svc->count; ++i) {
        if ((svc->errors[i].id &&
             strcmp(svc->errors[i].id, id_or_code) == 0) ||
            (svc->errors[i].error_code &&
             strcmp(svc->errors[i].error_code, id_or_code) == 0)) {
            return &svc->errors[i];
        }
    }
    return NULL;
}

static bool my_error_exists(TifErrorService* self, const char* id_or_code) {
    return my_error_get(self, id_or_code) != NULL;
}

static void my_error_destroy(TifService* self) {
    (void)self;
}
```

### Wiring

```c
MyErrorService error_svc;
error_svc.vtable.base.destroy = my_error_destroy;
error_svc.vtable.get          = my_error_get;
error_svc.vtable.exists       = my_error_exists;
error_svc.errors              = errors_array;
error_svc.count               = errors_count;

tif_app_ctx_init(&app, ..., (TifErrorService*)&error_svc, ...);
```

## Heap-Allocated vs. Stack-Allocated Services

The examples above use stack-allocated (or statically-allocated) services with no-op `destroy` functions. For heap-allocated services (e.g., a future YAML repository that allocates memory during `init`):

```c
static void yaml_feature_svc_destroy(TifService* self) {
    YamlFeatureService* svc = (YamlFeatureService*)self;
    // Free any owned TifFeature objects loaded from YAML.
    for (size_t i = 0; i < svc->count; ++i) {
        tif_feature_destroy(&svc->features[i]);
    }
    free(svc->features);
    free(svc);  // Free the struct itself if heap-allocated.
}
```

The `TifDIContainer` does **not** call `destroy` on the feature/error services passed to `tif_app_ctx_init` — those are caller-managed. Only services resolved through the DI container have `destroy` called by `tif_di_destroy`.

## Key Rules

- The interface struct (`TifFeatureService`, `TifErrorService`) must be the **first member** of the concrete struct.
- Always implement `base.destroy` — use a no-op for stack/static allocations.
- Pass the address of your concrete struct cast to the interface pointer type.
- The feature service's `get` returns a borrowed pointer into the service's storage — callers must not free it.

## Related Documentation

- [`docs/core/interfaces.md`](../core/interfaces.md) — vtable pattern and first-member casting rules
- [`docs/guides/domain/error.md`](domain/error.md) — building `TifError` objects
- [`docs/guides/domain/feature.md`](domain/feature.md) — building `TifFeature` objects
- [`docs/guides/contexts.md`](contexts.md) — full pipeline wiring
- [`examples/calculator/main.c`](../../examples/calculator/main.c) — complete in-memory service implementations
