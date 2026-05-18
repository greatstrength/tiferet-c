# Mappers in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

The mappers layer bridges persistent configuration and runtime domain objects. It provides two patterns:

1. **Aggregate** — wraps a domain struct and adds mutation functions.
2. **TransferObject (YamlObject)** — wraps a domain struct and adds serialization functions (`to_map`, `map`, `from_model`).

In Python and C++, these use class inheritance. In C, they use **composition**: a wrapper struct embeds the domain struct as a field named `data`.

## The Aggregate Pattern

An aggregate embeds the domain struct as `data` and exposes mutation functions:

```c
// ** mapper: tif_error_aggregate
typedef struct {
    TifError data;  // Embedded domain object.
} TifErrorAggregate;
```

### Mutation Functions

Aggregates provide named mutation operations that validate inputs before updating fields:

```c
// Rename: free old name, copy new one.
TifResult tif_error_agg_rename(TifErrorAggregate* agg, const char* name);

// Set an attribute by name — returns error for unknown attributes.
TifResult tif_error_agg_set_attribute(TifErrorAggregate* agg,
                                      const char* attribute,
                                      const char* value);

// Add a localized message.
TifResult tif_error_agg_add_message(TifErrorAggregate* agg,
                                    const char* lang,
                                    const char* text);
```

`set_attribute` mirrors Python `Aggregate.set_attribute` — it dispatches on the attribute name and raises `TIF_ERR_INVALID_MODEL_ATTRIBUTE` for unknown fields:

```c
TifResult tif_error_agg_set_attribute(TifErrorAggregate* agg,
                                      const char* attribute,
                                      const char* value) {
    if (strcmp(attribute, "name") == 0) {
        free(agg->data.name);
        agg->data.name = tif_strdup(value);
        return TIF_OK;
    }
    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}
```

### Lifecycle

Aggregates are initialised and destroyed using the same pattern as domain objects:

```c
TifResult tif_error_agg_init(TifErrorAggregate* agg, const char* id,
                             const char* name, const char* description,
                             const char* error_code);
void tif_error_agg_destroy(TifErrorAggregate* agg);
```

These delegate directly to the embedded domain struct's lifecycle functions.

## The TransferObject Pattern

A transfer object (named `<Domain>YamlObject`) also embeds the domain struct as `data` and adds three serialization operations:

```c
// ** mapper: tif_error_yaml_object
typedef struct {
    TifError data;  // Embedded domain object.
} TifErrorYamlObject;
```

### `to_map` — Serialize to `TifMap`

Converts the transfer object's fields into a `TifMap` (string→string) for serialization:

```c
TifResult tif_error_yaml_to_map(const TifErrorYamlObject* obj,
                                TifMap* out,
                                const char* role);
```

The `role` parameter selects which fields to include (e.g., `"to_data.yaml"` excludes `id`; `"to_model"` excludes `message`).

### `map` — Convert to Aggregate

Maps the transfer object into an aggregate — the primary construction path from configuration data:

```c
TifResult tif_error_yaml_map(const TifErrorYamlObject* obj,
                             TifErrorAggregate* out);
```

### `from_model` — Convert from Domain Object

Creates a transfer object from an existing runtime domain struct — used when serialising back to disk:

```c
TifResult tif_error_yaml_from_model(const TifError* model,
                                    TifErrorYamlObject* out);
```

## Complete Example: `TifErrorAggregate`

```c
// Initialise
TifErrorAggregate agg;
TIF_PROPAGATE(tif_error_agg_init(&agg, "division_by_zero",
                                 "Division By Zero", NULL, NULL));

// Mutate
TIF_PROPAGATE(tif_error_agg_set_attribute(&agg, "name", "Divide By Zero"));

// Access via embedded data field
printf("Code: %s\n", agg.data.error_code);  // "DIVISION_BY_ZERO"

// Destroy
tif_error_agg_destroy(&agg);
```

## Naming Convention

| Component | Pattern | Example |
|---|---|---|
| Aggregate struct | `Tif<Domain>Aggregate` | `TifErrorAggregate` |
| Aggregate init | `tif_<domain>_agg_init` | `tif_error_agg_init` |
| Aggregate mutation | `tif_<domain>_agg_<action>` | `tif_error_agg_rename` |
| TransferObject struct | `Tif<Domain>YamlObject` | `TifErrorYamlObject` |
| TransferObject to_map | `tif_<domain>_yaml_to_map` | `tif_error_yaml_to_map` |
| TransferObject map | `tif_<domain>_yaml_map` | `tif_error_yaml_map` |
| TransferObject from_model | `tif_<domain>_yaml_from_model` | `tif_error_yaml_from_model` |

## Best Practices

- Aggregates access and mutate the domain struct via the `data` field — never bypass it.
- `set_attribute` must raise `TIF_ERR_INVALID_MODEL_ATTRIBUTE` for unknown attribute names.
- TransferObjects are used only for serialization/deserialization — not for runtime domain logic.
- `map` (TransferObject → Aggregate) is the primary path from raw configuration to runtime models.
- `from_model` (Domain → TransferObject) is the primary path for persistence (write-back).

## Package Layout

```
include/tiferet/mappers/
├── settings.h     — documentation-only: describes aggregate/transfer object conventions
├── error.h        — TifErrorAggregate, TifErrorYamlObject
├── feature.h      — TifFeatureAggregate, TifFeatureYamlObject (+ TifFeatureEventAggregate/YamlObject)
├── app.h          — TifAppInterfaceAggregate, TifAppInterfaceYamlObject
└── di.h           — TifServiceConfigAggregate, TifServiceConfigYamlObject

src/mappers/
├── error.c
├── feature.c
├── app.c
└── di.c
```

## Conclusion

The mappers layer provides the structural bridge between persistent configuration (YAML) and runtime domain objects in tiferet-c. Using C composition instead of C++ inheritance, aggregates add mutation to domain structs and transfer objects add serialization — maintaining a clean separation without duplicating field definitions.

Explore `include/tiferet/mappers/` and `src/mappers/` for implementation details.
