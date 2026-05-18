# Events Guide – Implementing TifExecutableEvent

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

`TifExecutableEvent` is the vtable interface that feature pipeline steps must implement. Every domain event in a tiferet-c application is a concrete struct that:

1. Embeds `TifExecutableEvent` as its first member (enabling safe cast to `TifService*`).
2. Implements a `destroy` function that frees any owned resources.
3. Implements an `execute` function that reads inputs, performs domain logic, and returns a result.
4. Provides a factory function that allocates and wires the struct (registered in `TifServiceRegistry`).

## The Four-Part Pattern

### 1. Define the Event Struct

```c
// A domain event for adding two numbers.
// The vtable field must be first — enables safe cast to TifService* and TifExecutableEvent*.
typedef struct {
    TifExecutableEvent vtable;
    // Add owned state here if needed (e.g., injected service pointers).
} AddCalcEvent;
```

For stateless events (no injected dependencies), no additional fields are needed.

### 2. Implement `destroy`

```c
static void add_event_destroy(TifService* self) {
    free(self);  // Free the heap-allocated struct.
    // If the struct holds owned pointers, free them first.
}
```

The `destroy` function receives a `TifService*`. Cast to your concrete type if you need to free owned fields.

### 3. Implement `execute`

```c
static TifResult add_event_execute(TifExecutableEvent* self,
                                   const TifMap* data,
                                   const TifMap* params,
                                   char** result_out) {
    (void)self;  // Suppress unused-parameter warning for stateless events.

    // Read inputs: check params first, then data (params take priority).
    const char* a_str = (const char*)tif_map_get(params, "a");
    if (!a_str) a_str = (const char*)tif_map_get(data, "a");

    const char* b_str = (const char*)tif_map_get(params, "b");
    if (!b_str) b_str = (const char*)tif_map_get(data, "b");

    // Convert to numeric values.
    double a = a_str ? strtod(a_str, NULL) : 0.0;
    double b = b_str ? strtod(b_str, NULL) : 0.0;

    // Perform domain logic.
    double result = a + b;

    // Allocate and return the result string (caller takes ownership).
    *result_out = (char*)malloc(64);
    if (!*result_out) return TIF_ERR("ALLOC_FAILED");
    snprintf(*result_out, 64, "%g", result);

    return TIF_OK;
}
```

**Result contract:**
- If the event produces output: heap-allocate a string and set `*result_out` to it. The `TifFeatureContext` takes ownership and stores it under `data_key`.
- If no output: leave `*result_out` as `NULL` (it is initialised to `NULL` before the call).
- On error: return early via `TIF_VERIFY` or `TIF_RAISE`. Any partial `*result_out` allocation should be freed before returning.

### 4. Implement the Factory Function

```c
static TifService* add_event_factory(const TifMap* params) {
    (void)params;  // Suppress warning if params aren't used.

    AddCalcEvent* evt = (AddCalcEvent*)malloc(sizeof(AddCalcEvent));
    if (!evt) return NULL;

    // Wire the vtable.
    evt->vtable.base.destroy = add_event_destroy;
    evt->vtable.execute      = add_event_execute;

    return (TifService*)evt;
}
```

The factory receives the configuration's `parameters` map (from `TifServiceConfiguration.parameters`). Use it to configure the event if needed (e.g., precision, mode flags).

## Registering the Event

```c
TifServiceRegistry registry;
tif_registry_init(&registry);

tif_registry_register(&registry, "calc.events::AddEvent", add_event_factory);
```

The key `"calc.events::AddEvent"` must match the `module_path::class_name` in the `TifServiceConfiguration`.

## Using an Injected Service

If your event depends on a service (e.g., a database or file service), store it as a borrowed pointer in the event struct:

```c
typedef struct {
    TifExecutableEvent vtable;
    MyDataService*     data_svc;  // Borrowed — do NOT free in destroy.
} QueryEvent;

static void query_event_destroy(TifService* self) {
    free(self);  // Only free the struct, not the borrowed service.
}

static TifService* query_event_factory(const TifMap* params) {
    (void)params;
    QueryEvent* evt = (QueryEvent*)malloc(sizeof(QueryEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = query_event_destroy;
    evt->vtable.execute      = query_event_execute;
    evt->data_svc            = NULL;  // Set via setter or during init.
    return (TifService*)evt;
}
```

For dependency injection via factory params, read from the `params` map:

```c
static TifService* query_event_factory(const TifMap* params) {
    // params may carry config values like "table_name"
    const char* table = (const char*)tif_map_get(params, "table");
    // ...
}
```

## Error Guards

Use `TIF_VERIFY` for domain rule checks and `TIF_RAISE` for unconditional errors:

```c
static TifResult divide_event_execute(TifExecutableEvent* self,
                                      const TifMap* data,
                                      const TifMap* params,
                                      char** result_out) {
    (void)self;
    double a = /* ... */;
    double b = /* ... */;

    // Domain rule: denominator must be non-zero.
    TIF_VERIFY(b != 0.0, TIF_ERR_DIVISION_BY_ZERO, "Cannot divide by zero");

    *result_out = /* ... format a/b ... */;
    return TIF_OK;
}
```

When `TIF_VERIFY` fails, it does an early return before `*result_out` is set. The `TifFeatureContext` initialises `result_out` to `NULL` before calling `execute`, so the `NULL` value is handled safely.

## Complete Event Example

```c
#include <tiferet/tiferet.h>
#include <stdio.h>
#include <stdlib.h>

// ** event: multiply_calc_event

typedef struct { TifExecutableEvent vtable; } MultiplyCalcEvent;

static void multiply_destroy(TifService* self) { free(self); }

static TifResult multiply_execute(TifExecutableEvent* self,
                                  const TifMap* data,
                                  const TifMap* params,
                                  char** result_out) {
    (void)self;
    const char* as = (const char*)tif_map_get(params, "a");
    if (!as) as = (const char*)tif_map_get(data, "a");
    const char* bs = (const char*)tif_map_get(params, "b");
    if (!bs) bs = (const char*)tif_map_get(data, "b");

    double a = as ? strtod(as, NULL) : 0.0;
    double b = bs ? strtod(bs, NULL) : 0.0;

    *result_out = (char*)malloc(64);
    if (!*result_out) return TIF_ERR("ALLOC_FAILED");
    snprintf(*result_out, 64, "%g", a * b);
    return TIF_OK;
}

static TifService* multiply_factory(const TifMap* params) {
    (void)params;
    MultiplyCalcEvent* evt = (MultiplyCalcEvent*)malloc(sizeof(MultiplyCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = multiply_destroy;
    evt->vtable.execute      = multiply_execute;
    return (TifService*)evt;
}
```

## Checklist

- `TifExecutableEvent vtable` is the **first** field of the event struct.
- `destroy` frees the struct and any owned fields (not borrowed pointers).
- `execute` sets `*result_out` to a heap-allocated string, or leaves it `NULL`.
- `execute` frees any partial `*result_out` before an early error return.
- Factory uses `(void)params` if params aren't used (keeps build warning-free).
- Registry key matches `"module_path::class_name"` in `TifServiceConfiguration`.

## Related Documentation

- [`docs/core/interfaces.md`](../core/interfaces.md) — vtable pattern and safe casting
- [`docs/core/contexts.md`](../core/contexts.md) — `TifExecutableEvent` and `TifFeatureContext`
- [`docs/guides/domain/feature.md`](domain/feature.md) — feature steps and `service_id`
- [`docs/guides/contexts.md`](contexts.md) — full pipeline wiring
- [`examples/calculator/main.c`](../../examples/calculator/main.c) — four working event implementations
