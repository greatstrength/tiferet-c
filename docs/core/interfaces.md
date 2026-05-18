# Service Interfaces in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

Service interfaces in tiferet-c are C structs whose fields are function pointers — commonly called *vtables*. They provide the same abstract service contracts as `Service(ABC)` in the Python framework and `class Service` in C++, without requiring C++ virtual dispatch or any language extensions.

All interface structs embed `TifService base` as their **first member**, enabling safe upcasting from a concrete implementation to the base service pointer.

## The `TifService` Base Vtable

Defined in `include/tiferet/interfaces/settings.h`:

```c
typedef struct TifService {
    // Destroy this service instance, freeing all owned resources.
    // Heap-allocated implementations must also free the struct itself.
    void (*destroy)(struct TifService* self);
} TifService;
```

Every service must set `base.destroy` to a function that properly cleans up its resources. `TifDIContainer` calls `svc->destroy(svc)` on teardown.

## The Vtable Pattern

Service interfaces extend `TifService` by embedding it as the first member, then adding domain-specific function pointers:

```c
// ** interface: tif_feature_service
typedef struct TifFeatureService {
    TifService base;    // Must be first.

    const TifFeature* (*get)(struct TifFeatureService* self, const char* id);
    bool (*exists)(struct TifFeatureService* self, const char* id);
} TifFeatureService;
```

Concrete implementations embed the interface struct as their **first member**, enabling safe casting:

```c
// Concrete in-memory feature service
typedef struct {
    TifFeatureService vtable;   // Must be first.
    TifFeature*       features;
    size_t            count;
} MyFeatureService;

// Safe cast: TifFeatureService is the first member of MyFeatureService
TifFeatureService* svc = (TifFeatureService*)&my_feature_svc;
svc->get(svc, "calc.add");
```

This is identical to the way C++ virtual dispatch works internally; the cast is well-defined because the first member shares the same address as the enclosing struct.

## Calling a Service

All function pointers receive `self` as their first argument:

```c
// Look up a feature
const TifFeature* feat = feature_svc->get(feature_svc, "calc.add");

// Check existence
bool exists = error_svc->exists(error_svc, "DIVISION_BY_ZERO");
```

## Domain Service Interfaces

All defined in `include/tiferet/interfaces/` (header-only — no `.c` files).

### `TifFeatureService` (`interfaces/feature.h`)

```c
typedef struct TifFeatureService {
    TifService base;
    const TifFeature* (*get)(struct TifFeatureService* self, const char* id);
    bool (*exists)(struct TifFeatureService* self, const char* id);
} TifFeatureService;
```

### `TifErrorService` (`interfaces/error.h`)

```c
typedef struct TifErrorService {
    TifService base;
    const TifError* (*get)(struct TifErrorService* self, const char* id_or_code);
    bool (*exists)(struct TifErrorService* self, const char* id_or_code);
} TifErrorService;
```

Accepts either the YAML key (e.g., `"division_by_zero"`) or the derived uppercase code (e.g., `"DIVISION_BY_ZERO"`).

### `TifContainerService` and `TifAppService`

Container and app-level service contracts consumed by the DI and app layers.

## Implementing a Service

### 1. Define a concrete struct embedding the interface

```c
typedef struct {
    TifFeatureService vtable;   // First member — enables safe cast.
    TifFeature*       features;
    size_t            count;
} InMemoryFeatureService;
```

### 2. Implement the function pointers

```c
static const TifFeature* in_memory_feature_get(TifFeatureService* self,
                                               const char* id) {
    InMemoryFeatureService* svc = (InMemoryFeatureService*)self;
    for (size_t i = 0; i < svc->count; ++i) {
        if (strcmp(svc->features[i].id, id) == 0)
            return &svc->features[i];
    }
    return NULL;
}

static bool in_memory_feature_exists(TifFeatureService* self, const char* id) {
    return in_memory_feature_get(self, id) != NULL;
}

static void in_memory_feature_destroy(TifService* self) {
    (void)self; // Stack-allocated — nothing to free.
}
```

### 3. Wire the vtable

```c
InMemoryFeatureService svc;
svc.vtable.base.destroy = in_memory_feature_destroy;
svc.vtable.get          = in_memory_feature_get;
svc.vtable.exists       = in_memory_feature_exists;
svc.features            = features_array;
svc.count               = 4;

// Use as TifFeatureService* anywhere:
tif_app_ctx_init(&app, (TifFeatureService*)&svc, ...);
```

## Interfaces are Header-Only

Interface headers declare only vtable struct typedefs — no function declarations for lifecycle management, because concrete implementations own their own lifecycle. There are no `.c` files in `interfaces/`.

## Best Practices

- Always embed `TifService base` (or a derived interface) as the **first member** of concrete structs.
- Always implement and set `base.destroy` — even for stack-allocated services that do nothing (use `(void)self;`).
- Pass `self` as the first argument in all function pointer calls.
- Cast to the interface pointer type when passing to context functions: `(TifFeatureService*)&svc`.
- Never expose concrete struct internals across module boundaries — only the interface pointer.

## Package Layout

```
include/tiferet/interfaces/   (all header-only — no .c files)
├── settings.h    — TifService base vtable
├── feature.h     — TifFeatureService (get, exists)
├── error.h       — TifErrorService (get, exists)
├── container.h   — TifContainerService
└── app.h         — TifAppService
```

## Conclusion

The vtable pattern makes tiferet-c's service interfaces fully abstract in pure C11. Any struct that embeds the correct interface as its first member and wires the function pointers satisfies the contract — enabling the same dependency inversion and testability as the Python `Service(ABC)` or C++ `class Service` equivalents.

Explore `include/tiferet/interfaces/` and `examples/calculator/main.c` for implementation details.
