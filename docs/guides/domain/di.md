# Domain Guide – Dependency Injection

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

`TifServiceConfiguration` is the wiring record that connects a feature step to its concrete implementation. It maps a logical `id` (referenced by `TifFeatureEvent.service_id`) to a `module_path` and `class_name` — the registry key the `TifDIContainer` uses to look up the factory function.

`TifFlaggedDependency` adds flag-based override capability: when a specific feature flag is active, the container resolves a different `module_path`/`class_name` for the same `id`.

## `TifServiceConfiguration`

### Creating a Configuration

```c
#include <tiferet/tiferet.h>

TifServiceConfiguration cfg;
TifResult r = tif_service_config_init(
    &cfg,
    "add_event",        // id — must match TifFeatureEvent.service_id
    NULL,               // name (optional)
    "calc.events",      // module_path
    "AddEvent"          // class_name
);
// Registry key: "calc.events::AddEvent"
```

The `TifDIContainer` builds the registry key via `tif_str_make_key(module_path, class_name)` and calls `tif_registry_create` to create the live service.

### Default Parameters

Configurations can carry default parameters passed to the factory function:

```c
tif_map_set(&cfg.parameters, "precision", tif_strdup("2"));
// Factory receives params map with "precision" → "2"
```

### Destroying a Configuration

```c
tif_service_config_destroy(&cfg);
```

## `TifFlaggedDependency`

A flagged dependency provides an alternative implementation when a feature flag is active. This enables environment-specific overrides (e.g., `"test"` flag uses a stub, `"prod"` uses the real implementation) without changing feature definitions.

### Adding a Flagged Dependency

```c
TifFlaggedDependency dep;
TifResult r = tif_flagged_dep_init(
    &dep,
    "calc.stubs",       // module_path (override)
    "AddEventStub",     // class_name (override)
    "test"              // flag name
);

// Transfer ownership into the configuration.
tif_service_config_add_dependency(&cfg, dep);
// Do NOT call tif_flagged_dep_destroy on dep after this.
```

### Flag Resolution at Runtime

`TifDIContainer` scans the flags array in priority order. The first matching flagged dependency wins; the default `module_path`/`class_name` is only used if no flag matches:

```c
const char* flags[] = {"test"};
TifDIContainer di;
tif_di_init(&di, &cfg, 1, flags, 1, &registry);

// With "test" flag active:
//   registry key = "calc.stubs::AddEventStub"
// Without flags (NULL, 0):
//   registry key = "calc.events::AddEvent"
```

### Multiple Flags (Priority Order)

```c
// Higher-priority flags first.
const char* flags[] = {"canary", "staging"};
tif_di_init(&di, configs, config_count, flags, 2, &registry);
// "canary" is checked first; if no match, "staging" is checked; then default.
```

## Complete Example

```c
// 1. Create configurations.
TifServiceConfiguration configs[2];

tif_service_config_init(&configs[0], "add_event",    NULL, "calc.events", "AddEvent");
tif_service_config_init(&configs[1], "divide_event", NULL, "calc.events", "DivideEvent");

// 2. Add flagged dependency for testing.
{
    TifFlaggedDependency dep;
    tif_flagged_dep_init(&dep, "calc.stubs", "DivideEventStub", "test");
    tif_service_config_add_dependency(&configs[1], dep);
}

// 3. Build DI container (no flags in production).
TifDIContainer container;
tif_di_init(&container, configs, 2, NULL, 0, &registry);

// 4. With test flag.
TifDIContainer test_container;
const char* test_flags[] = {"test"};
tif_di_init(&test_container, configs, 2, test_flags, 1, &registry);
// configs[1] resolves to "calc.stubs::DivideEventStub"

// 5. Cleanup.
tif_di_destroy(&container);
tif_di_destroy(&test_container);
for (int i = 0; i < 2; ++i) {
    tif_service_config_destroy(&configs[i]);
}
```

## YAML Configuration Equivalent

```yaml
# container.yml (C++ / Python style)
attrs:
  add_event:
    module_path: calc.events
    class_name: AddEvent
  divide_event:
    module_path: calc.events
    class_name: DivideEvent
    flagged:
      - flag: test
        module_path: calc.stubs
        class_name: DivideEventStub
```

## Related Documentation

- [`docs/core/contexts.md`](../../core/contexts.md) — `TifDIContainer` API reference
- [`docs/guides/domain/feature.md`](feature.md) — `TifFeature` and step `service_id`
- [`docs/guides/contexts.md`](../contexts.md) — full pipeline wiring
