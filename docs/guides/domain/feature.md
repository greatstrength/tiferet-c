# Domain Guide – Feature

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

A `TifFeature` is a named, ordered workflow composed of steps (`TifFeatureEvent`). It defines *what* the application does. Each step references a service in the DI container and carries orchestration metadata: parameters, result routing (`data_key`), and error-continue behaviour (`pass_on_error`).

The `TifFeatureContext` loads a feature by ID and executes its steps in sequence, storing results in the data context map.

## Building a `TifFeature`

### 1. Initialise the Feature

```c
#include <tiferet/tiferet.h>

TifFeature feat;
TifResult r = tif_feature_init(
    &feat,
    "calc.add",         // id (NULL = derived from group_id + feature_key)
    "Add Number",       // name
    "calc",             // group_id
    "add",              // feature_key (NULL = derived from name via snake_case)
    "Adds one number to another"  // description (NULL = defaults to name)
);
```

If `id` is `NULL`, it is derived as `group_id + "." + feature_key`. If `feature_key` is also `NULL`, it is derived from `name` via `tif_str_to_snake`.

### 2. Create and Add Steps

```c
TifFeatureEvent step;
TifResult r = tif_feature_event_init(
    &step,
    "Add a and b",      // name
    "add_event",        // service_id — matches a TifServiceConfiguration id
    "result",           // data_key — stores the result in data["result"]
    false,              // pass_on_error
    NULL                // condition (NULL = always execute)
);

// tif_feature_add_step takes OWNERSHIP of the step via shallow copy.
// Do NOT call tif_feature_event_destroy on `step` after this call.
tif_feature_add_step(&feat, step);
```

**Ownership transfer:** `tif_feature_add_step` shallow-copies `step` into the internal heap array. The caller must not destroy the source `TifFeatureEvent` afterward — `tif_feature_destroy` will handle cleanup via the array.

### 3. Destroy the Feature

```c
tif_feature_destroy(&feat);
// Frees id, name, description, group_id, feature_key, and all steps.
```

## Step Parameters

Step parameters are key-value pairs (string→string) that override or supplement data context values when the step executes. The `TifFeatureContext` passes them to the event's `execute` function as the `params` map.

```c
TifFeatureEvent step;
tif_feature_event_init(&step, "Raise to power", "exp_event", "result", false, NULL);

// Add a static parameter: b = "0.5" (square root)
tif_map_set(&step.parameters, "b", tif_strdup("0.5"));

tif_feature_add_step(&feat, step);
```

**Important:** Parameter values stored in `TifFeatureEvent.parameters` are heap-allocated strings owned by the map. They are freed when the step is destroyed (via `tif_feature_destroy`).

Events read parameters with params-first lookup:

```c
// Checks params first, then falls back to data.
const char* b = (const char*)tif_map_get(params, "b");
if (!b) b = (const char*)tif_map_get(data, "b");
```

## The `data_key` Field

When `data_key` is set, `TifFeatureContext` stores the event's result string under that key in the data map after execution. Subsequent steps can then read it from the data map.

```yaml
# Equivalent YAML config:
# commands:
#   - service_id: add_event
#     data_key: sum
#   - service_id: double_event
#     params:
#       a: $r.sum    # reads result of previous step (Python only; in C: read from data map)
```

In C, downstream steps simply call `tif_map_get(data, "sum")` to retrieve the earlier step's result.

## `pass_on_error`

When `pass_on_error` is `true`, an error from that step is recorded but execution continues to the next step. Use this for optional or non-critical steps:

```c
TifFeatureEvent logging_step;
tif_feature_event_init(&logging_step, "Log result", "log_event",
                       NULL, true, NULL);  // pass_on_error = true
```

## Multi-Step Feature

```c
TifFeature pipeline;
tif_feature_init(&pipeline, "calc.add_and_double", "Add then Double",
                 "calc", "add_and_double", NULL);

// Step 1: add a + b, store in "sum"
{
    TifFeatureEvent s;
    tif_feature_event_init(&s, "Add a and b", "add_event", "sum", false, NULL);
    tif_feature_add_step(&pipeline, s);
}

// Step 2: multiply sum * 2, store in "result"
{
    TifFeatureEvent s;
    tif_feature_event_init(&s, "Double the sum", "multiply_event", "result", false, NULL);
    tif_map_set(&s.parameters, "b", tif_strdup("2"));
    tif_feature_add_step(&pipeline, s);
}
```

## Retrieving a Step

```c
const TifFeatureEvent* step = tif_feature_get_step(&feat, 0);  // first step
const TifFeatureEvent* oob  = tif_feature_get_step(&feat, 99); // NULL (out of range)
```

## YAML Configuration Equivalent

```yaml
features:
  calc:
    add:
      name: Add Number
      description: Adds one number to another
      commands:
        - service_id: add_event
          name: Add a and b
          data_key: result
    sqrt:
      name: Square Root
      commands:
        - service_id: exp_event
          name: Raise to 0.5
          data_key: result
          params:
            b: '0.5'
```

## Related Documentation

- [`docs/core/domain.md`](../../core/domain.md) — domain object conventions and ownership rules
- [`docs/guides/events.md`](../events.md) — implementing `TifExecutableEvent`
- [`docs/guides/domain/di.md`](di.md) — service configuration and DI container
- [`docs/guides/contexts.md`](../contexts.md) — full pipeline wiring
