# Contexts Guide – Wiring the Full Pipeline

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

This guide walks through building a complete tiferet-c application from scratch — from domain objects and service implementations to the full runtime pipeline. It mirrors what `examples/calculator/main.c` does, explained step by step.

The six stages are:

1. Build domain objects (features, errors)
2. Implement services (`TifFeatureService`, `TifErrorService`)
3. Define service configurations
4. Wire the registry
5. Build the DI container and app context
6. Run features and handle errors

## Stage 1 — Build Domain Objects

### Features

```c
TifFeature features[2];

// Feature 1: calc.add
tif_feature_init(&features[0], "calc.add", "Add Number", "calc", "add", NULL);
{
    TifFeatureEvent step;
    tif_feature_event_init(&step, "Add a and b", "add_event", "result", false, NULL);
    tif_feature_add_step(&features[0], step);  // Ownership transferred.
}

// Feature 2: calc.divide
tif_feature_init(&features[1], "calc.divide", "Divide Number", "calc", "divide", NULL);
{
    TifFeatureEvent step;
    tif_feature_event_init(&step, "Divide a by b", "divide_event", "result", false, NULL);
    tif_feature_add_step(&features[1], step);
}
```

### Errors

```c
TifError errors[1];
tif_error_init(&errors[0], "division_by_zero", "Division By Zero", NULL, NULL);
tif_error_add_message(&errors[0], "en_US", "Cannot divide by zero");
```

## Stage 2 — Implement Services

```c
// Feature service (in-memory, stack-allocated).
typedef struct { TifFeatureService vtable; TifFeature* features; size_t count; } MyFeatureSvc;
static const TifFeature* feat_get(TifFeatureService* s, const char* id) {
    MyFeatureSvc* svc = (MyFeatureSvc*)s;
    for (size_t i = 0; i < svc->count; ++i)
        if (strcmp(svc->features[i].id, id) == 0) return &svc->features[i];
    return NULL;
}
static bool feat_exists(TifFeatureService* s, const char* id) { return feat_get(s, id) != NULL; }
static void feat_destroy(TifService* s) { (void)s; }

MyFeatureSvc feature_svc;
feature_svc.vtable.base.destroy = feat_destroy;
feature_svc.vtable.get          = feat_get;
feature_svc.vtable.exists       = feat_exists;
feature_svc.features            = features;
feature_svc.count               = 2;

// Error service (similar pattern, omitted for brevity).
// See examples/calculator/main.c for the full implementation.
```

## Stage 3 — Define Service Configurations

Each configuration links a logical `id` (used in feature steps as `service_id`) to the registry key:

```c
TifServiceConfiguration configs[2];
tif_service_config_init(&configs[0], "add_event",    NULL, "my.events", "AddEvent");
tif_service_config_init(&configs[1], "divide_event", NULL, "my.events", "DivideEvent");
// Registry keys: "my.events::AddEvent", "my.events::DivideEvent"
```

## Stage 4 — Wire the Registry

Register a factory function for each event type:

```c
TifServiceRegistry registry;
tif_registry_init(&registry);

tif_registry_register(&registry, "my.events::AddEvent",    add_event_factory);
tif_registry_register(&registry, "my.events::DivideEvent", divide_event_factory);
```

See [`docs/guides/events.md`](events.md) for how to implement `add_event_factory`.

## Stage 5 — Build DI Container and App Context

```c
// DI container resolves configs → live services via registry.
TifDIContainer container;
TifResult r = tif_di_init(&container, configs, 2, NULL, 0, &registry);
if (TIF_IS_ERR(r)) { /* handle init failure */ }

// App context ties everything together.
TifAppContext app;
tif_app_ctx_init(&app,
    (TifFeatureService*)&feature_svc,
    (TifErrorService*)&error_svc,
    &container);
```

## Stage 6 — Run Features

### Success Path

```c
TifMap data;
tif_map_init(&data);
tif_map_set(&data, "a", (void*)"10");
tif_map_set(&data, "b", (void*)"5");

TifResult r = tif_app_ctx_run(&app, "calc.add", &data);
if (TIF_IS_OK(r)) {
    char* result = (char*)tif_map_get(&data, "result");
    printf("10 + 5 = %s\n", result);  // "15"
    free(result);  // Free the heap-allocated result string.
}
tif_map_destroy(&data);
```

### Error Path

```c
TifMap data2;
tif_map_init(&data2);
tif_map_set(&data2, "a", (void*)"8");
tif_map_set(&data2, "b", (void*)"0");

TifResult r = tif_app_ctx_run(&app, "calc.divide", &data2);
if (TIF_IS_ERR(r)) {
    char msg[256] = {0};
    int n = tif_error_ctx_format(tif_app_ctx_error_ctx(&app),
                                  r.error_code, "en_US",
                                  msg, sizeof(msg));
    if (n <= 0 && r.message)
        snprintf(msg, sizeof(msg), "%s", r.message);

    printf("Error: %s\n", msg);  // "Error: Cannot divide by zero"
}
tif_map_destroy(&data2);
```

## Stage 7 — Cleanup

**Order matters:** destroy the DI container before the registry (container holds live services; registry holds factories).

```c
tif_di_destroy(&container);         // Calls svc->destroy on all resolved services.
tif_registry_destroy(&registry);    // Frees factory wrappers.

for (int i = 0; i < 2; ++i) {
    tif_service_config_destroy(&configs[i]);
    tif_feature_destroy(&features[i]);
}
tif_error_destroy(&errors[0]);
// feature_svc and error_svc are stack-allocated — no explicit destroy needed.
```

## Reusing the Pipeline for Multiple Runs

`TifAppContext` is reusable. Build it once and call `tif_app_ctx_run` as many times as needed:

```c
// Each call is independent — data maps are created and destroyed per-call.
for (size_t i = 0; i < test_count; ++i) {
    TifMap data;
    tif_map_init(&data);
    // populate data...
    TifResult r = tif_app_ctx_run(&app, test_cases[i].feature_id, &data);
    // handle result...
    tif_map_destroy(&data);
}
```

## Using Feature Flags

Pass flags to `tif_di_init` to activate environment-specific service implementations:

```c
const char* flags[] = {"test"};
TifResult r = tif_di_init(&container, configs, 2, flags, 1, &registry);
// Any config with a flagged dependency matching "test" uses the override.
```

See [`docs/guides/domain/di.md`](domain/di.md) for the flagged dependency pattern.

## Quick Reference — Correct Cleanup Order

```
tif_di_destroy(&container)         // 1. Destroy DI container (calls svc->destroy on each service)
tif_registry_destroy(&registry)    // 2. Destroy registry (frees factory wrappers)
tif_service_config_destroy(...)    // 3. Destroy service configurations
tif_feature_destroy(...)           // 4. Destroy features (frees steps and all strings)
tif_error_destroy(...)             // 5. Destroy errors (frees messages and all strings)
```

## Related Documentation

- [`docs/core/contexts.md`](../core/contexts.md) — full API reference for all context types
- [`docs/guides/events.md`](events.md) — implementing `TifExecutableEvent`
- [`docs/guides/interfaces.md`](interfaces.md) — implementing `TifFeatureService` and `TifErrorService`
- [`docs/guides/domain/feature.md`](domain/feature.md) — building features
- [`docs/guides/domain/di.md`](domain/di.md) — service configurations and flags
- [`examples/calculator/main.c`](../../examples/calculator/main.c) — complete working example
