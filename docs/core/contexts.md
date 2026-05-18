# Runtime Contexts in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

The contexts layer is the runtime orchestration engine of tiferet-c. It provides five components that wire together the domain, interface, and mapper layers into a working application pipeline:

| Component | Purpose |
|---|---|
| `TifServiceRegistry` | Maps `"module::Class"` keys to factory functions |
| `TifDIContainer` | Resolves service configurations into live service instances |
| `TifExecutableEvent` | Vtable interface for feature pipeline steps |
| `TifFeatureContext` | Iterates and executes a feature's step pipeline |
| `TifErrorContext` | Looks up and formats error messages |
| `TifAppContext` | Top-level entry point: feature lookup + pipeline execution |

## `TifServiceRegistry`

Defined in `include/tiferet/contexts/registry.h`.

Maps `"module_path::class_name"` string keys to `TifFactoryFn` factory functions. Mirrors Python's dynamic `import_module` + `getattr` pattern.

```c
typedef TifService* (*TifFactoryFn)(const TifMap* params);
```

### Usage

```c
TifServiceRegistry registry;
tif_registry_init(&registry);

// Register a factory under "calc.events::AddEvent"
tif_registry_register(&registry, "calc.events::AddEvent", add_event_factory);

// Create a service instance
TifService* svc = NULL;
TifResult r = tif_registry_create(&registry, "calc.events::AddEvent", NULL, &svc);

// ... use svc ...
svc->destroy(svc);
tif_registry_destroy(&registry);
```

### Design Note

Factory functions are stored in heap-allocated `TifFactoryWrapper` structs rather than directly as `void*` in `TifMap`. This avoids ISO-C-prohibited casts between function pointers and object pointers, keeping the build warning-free under `-Wpedantic`.

### API

```c
void     tif_registry_init(TifServiceRegistry* reg);
void     tif_registry_destroy(TifServiceRegistry* reg);
bool     tif_registry_register(TifServiceRegistry* reg, const char* key, TifFactoryFn factory);
TifResult tif_registry_create(const TifServiceRegistry* reg, const char* key,
                               const TifMap* params, TifService** out);
bool     tif_registry_has(const TifServiceRegistry* reg, const char* key);
size_t   tif_registry_size(const TifServiceRegistry* reg);
```

## `TifDIContainer`

Defined in `include/tiferet/contexts/di.h`.

Resolves an array of `TifServiceConfiguration` entries into live `TifService*` instances using a `TifServiceRegistry`. For each configuration, it checks flagged dependencies first (in priority order), then falls back to the default `module_path`/`class_name`.

### Usage

```c
TifDIContainer container;
TifResult r = tif_di_init(&container, configs, config_count,
                           flags, flag_count, &registry);

// Retrieve a resolved service
TifService* svc = tif_di_get(&container, "add_event");

// Teardown: calls svc->destroy(svc) on every owned service
tif_di_destroy(&container);
```

### Ownership

`TifDIContainer` **owns** all resolved services. `tif_di_destroy` iterates the internal map and calls `svc->destroy(svc)` on each service before freeing the map.

### API

```c
TifResult  tif_di_init(TifDIContainer* di, const TifServiceConfiguration* configs,
                        size_t config_count, const char** flags, size_t flag_count,
                        const TifServiceRegistry* registry);
void       tif_di_destroy(TifDIContainer* di);
TifService* tif_di_get(const TifDIContainer* di, const char* id);
bool       tif_di_has(const TifDIContainer* di, const char* id);
size_t     tif_di_size(const TifDIContainer* di);
```

## `TifExecutableEvent`

Defined in `include/tiferet/contexts/feature.h`.

Extends `TifService` with an `execute` function pointer — the vtable interface for feature pipeline steps:

```c
typedef struct TifExecutableEvent {
    TifService base;   // Must be first.

    TifResult (*execute)(struct TifExecutableEvent* self,
                         const TifMap* data,
                         const TifMap* params,
                         char** result_out);
} TifExecutableEvent;
```

If the event produces a result, the implementation heap-allocates a string and sets `*result_out`. The caller (`TifFeatureContext`) takes ownership and stores it under `data_key` or frees it.

### Implementing an Event

```c
typedef struct { TifExecutableEvent vtable; } AddCalcEvent;

static void add_event_destroy(TifService* self) { free(self); }

static TifResult add_event_execute(TifExecutableEvent* self,
                                   const TifMap* data,
                                   const TifMap* params,
                                   char** result_out) {
    (void)self;
    double a = /* read "a" from params then data */;
    double b = /* read "b" from params then data */;
    *result_out = /* heap-allocate formatted result string */;
    return TIF_OK;
}

static TifService* add_event_factory(const TifMap* params) {
    (void)params;
    AddCalcEvent* evt = malloc(sizeof(AddCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = add_event_destroy;
    evt->vtable.execute      = add_event_execute;
    return (TifService*)evt;
}
```

## `TifFeatureContext`

Defined in `include/tiferet/contexts/feature.h`.

Iterates a `TifFeature`'s steps and executes them:

1. Resolves the service from `TifDIContainer` by `service_id`.
2. Casts to `TifExecutableEvent*`.
3. Calls `execute(data, params, &result)`.
4. Stores the result string under `data_key` (if both are present).
5. Propagates errors unless `pass_on_error` is set on the step.

Result strings stored in the data map are heap-allocated; the caller must free them after use.

### API

```c
void      tif_feature_ctx_init(TifFeatureContext* ctx, const TifDIContainer* container);
TifResult tif_feature_ctx_execute(TifFeatureContext* ctx,
                                  const TifFeature* feature,
                                  TifMap* data);
```

## `TifErrorContext`

Defined in `include/tiferet/contexts/error.h`.

A thin wrapper over `TifErrorService` for error lookup and message formatting:

```c
void            tif_error_ctx_init(TifErrorContext* ctx, TifErrorService* error_service);
const TifError* tif_error_ctx_get(const TifErrorContext* ctx, const char* id_or_code);
int             tif_error_ctx_format(const TifErrorContext* ctx, const char* id_or_code,
                                     const char* lang, char* buf, size_t buf_size);
bool            tif_error_ctx_exists(const TifErrorContext* ctx, const char* id_or_code);
```

`tif_error_ctx_format` returns the number of characters written, or 0 if the error or language is not found. `lang` defaults to `"en_US"` when `NULL`.

## `TifAppContext`

Defined in `include/tiferet/contexts/app.h`.

The top-level entry point. Embeds a `TifErrorContext` and a `TifFeatureContext` and exposes `tif_app_ctx_run`:

```c
void      tif_app_ctx_init(TifAppContext* ctx,
                           TifFeatureService* feature_service,
                           TifErrorService* error_service,
                           const TifDIContainer* container);
TifResult tif_app_ctx_run(TifAppContext* ctx,
                          const char* feature_id,
                          TifMap* data);
TifErrorContext* tif_app_ctx_error_ctx(TifAppContext* ctx);
```

`tif_app_ctx_run` looks up the feature by ID, returns `TIF_ERR_FEATURE_NOT_FOUND` if absent, then delegates to `tif_feature_ctx_execute`.

## Complete Wiring Example

```c
// 1. Registry
TifServiceRegistry registry;
tif_registry_init(&registry);
tif_registry_register(&registry, "calc.events::AddEvent", add_event_factory);

// 2. Service configurations
TifServiceConfiguration configs[1];
tif_service_config_init(&configs[0], "add_event", NULL, "calc.events", "AddEvent");

// 3. DI container
TifDIContainer container;
tif_di_init(&container, configs, 1, NULL, 0, &registry);

// 4. App context
TifAppContext app;
tif_app_ctx_init(&app, &feature_svc, &error_svc, &container);

// 5. Run
TifMap data;
tif_map_init(&data);
tif_map_set(&data, "a", (void*)"3");
tif_map_set(&data, "b", (void*)"4");

TifResult r = tif_app_ctx_run(&app, "calc.add", &data);
if (TIF_IS_OK(r)) {
    char* result = (char*)tif_map_get(&data, "result");
    printf("3 + 4 = %s\n", result);
    free(result);
}
tif_map_destroy(&data);

// 6. Teardown
tif_di_destroy(&container);
tif_registry_destroy(&registry);
tif_service_config_destroy(&configs[0]);
```

## Package Layout

```
include/tiferet/contexts/
├── registry.h   — TifServiceRegistry + TifFactoryFn
├── di.h         — TifDIContainer
├── feature.h    — TifExecutableEvent + TifFeatureContext
├── error.h      — TifErrorContext
└── app.h        — TifAppContext

src/contexts/
├── registry.c
├── di.c
├── feature.c
├── error.c
└── app.c
```

## Conclusion

The contexts layer completes the tiferet-c runtime stack. Starting from a registry of factory functions, it builds a fully wired DI container, executes feature pipelines step-by-step, and surfaces results through a simple `tif_app_ctx_run` call. See `examples/calculator/main.c` for a complete working demonstration.
