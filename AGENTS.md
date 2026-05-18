# AGENTS.md — tiferet-c (v1.0.0b1)

## Project Overview

**tiferet-c** is a pure C11 implementation of the Tiferet Domain-Driven Design framework. It provides a layered architecture for building applications with domain structs, vtable-based service interfaces, configuration-driven feature workflows, and dependency injection — all expressed in idiomatic C without mandatory external dependencies.

- **Repository:** https://github.com/greatstrength/tiferet-c
- **Branch:** `v1.0b1-release`
- **Language:** C11
- **Version:** `1.0.0b1`
- **Reference implementations:** Python (`tiferet`), C++ (`tiferet-cpp`), C# (`tiferet.net`)

## Architecture

### Layer Overview

```
tiferet-c/
├── include/tiferet/       # Public API — shape (structs), contracts (fn signatures), values (macros)
│   ├── tiferet.h          # Umbrella header: #include <tiferet/tiferet.h>
│   ├── assets/            # TifResult type, TIF_ERR_* error code constants
│   ├── domain/            # Domain structs: TifError, TifFeature, TifAppInterface, TifServiceConfiguration
│   ├── events/            # TIF_VERIFY / TIF_RAISE / TIF_PROPAGATE macros; tif_raise_error()
│   ├── interfaces/        # TifService vtable + domain service interfaces (header-only)
│   ├── mappers/           # Aggregate structs + transfer object structs + function declarations
│   ├── contexts/          # TifServiceRegistry, TifDIContainer, TifFeatureContext, TifAppContext
│   └── utils/             # TifMap, TifVector, strutil
└── src/                   # Implementations — mirrors include/ (no headers here)
    ├── assets/
    ├── domain/
    ├── events/
    ├── mappers/
    ├── contexts/
    └── utils/
```

### File Placement Rules

- **`include/`** — Shape, contract, value: struct layouts, function signatures, `#define` macros.
- **`src/`** — Behavior only: function bodies. No declarations here.
- **Interfaces are header-only** — pure vtable structs, no `.c` file needed.
- **Assets constants are header-only** — `#define` macros only.

### Key Concepts

**`TifResult`** (`assets/errors.h`): The universal return type for all fallible functions. `error_code == NULL` means success.

```c
typedef struct {
    const char* error_code;   // NULL on success
    const char* message;      // Optional human-readable message
} TifResult;
```

Use `TIF_OK`, `TIF_IS_OK(r)`, `TIF_IS_ERR(r)` macros. Never compare `error_code` directly.

**Domain objects** (`domain/`) are plain C structs with an `_init` / `_destroy` / `_validate` convention. All `char*` fields are heap-allocated (owned). `_init` copies inputs via `tif_strdup`; `_destroy` frees them.

**Vtable pattern** (`interfaces/`): Service interfaces are structs whose first member is `TifService base`. Concrete implementations embed the interface struct as their first member, enabling safe upcasting:

```c
typedef struct { TifService base; /* function pointers */ } TifFeatureService;
typedef struct { TifFeatureService vtable; TifFeature* features; size_t count; } MyFeatureService;
// Safe: (TifFeatureService*)&my_svc
```

**`TifDIContainer`** resolves `TifServiceConfiguration` entries into live `TifService*` instances via `TifServiceRegistry`. Owns all resolved services; calls `destroy` on cleanup.

**`TifExecutableEvent`** extends `TifService` with an `execute(self, data, params, &result_out)` function pointer. Domain events in feature pipelines must implement this interface.

**`TifAppContext`** is the top-level entry point: `tif_app_ctx_run(&app, "feature.id", &data)`.

### Runtime Flow

1. `tif_registry_init` + `tif_registry_register` — wire factory functions to `"module::Class"` keys.
2. `tif_di_init` — resolve `TifServiceConfiguration[]` into live services via registry.
3. `tif_app_ctx_init` — combine feature service, error service, and DI container.
4. `tif_app_ctx_run` — look up feature, iterate steps, resolve each from DI, cast to `TifExecutableEvent*`, call `execute`, store result under `data_key`.

## Structured Code Style

All code follows a strict artifact comment hierarchy. **This is mandatory.**

### Comment Levels

```c
// *** <section>      Top-level: imports, constants, types, models, events,
//                    interfaces, mappers, repos, utils, contexts, functions
// ** <category>: <name>   Mid-level: e.g. "model: tif_error", "util: tif_map"
// * <component>      Low-level: "attribute: <name>", "init", "method: <name>",
//                    "method: <name> (static)", "function: <name>"
```

### Spacing Rules

- One empty line between `// ***` and first `// **`.
- One empty line between `// **` sections.
- One empty line between `// *` sections and code blocks.
- One empty line between code snippets within a function.

### Naming Conventions

| Kind | Convention | Example |
|---|---|---|
| Types | `Tif` + PascalCase | `TifError`, `TifMap` |
| Functions | `tif_` + snake_case | `tif_error_init`, `tif_map_set` |
| Error codes | `TIF_ERR_` + SCREAMING_SNAKE | `TIF_ERR_FEATURE_NOT_FOUND` |
| Macros | `TIF_` + SCREAMING_SNAKE | `TIF_VERIFY`, `TIF_PROPAGATE` |

### Import Organization (headers)

```c
// *** includes

// ** core
#include <stdlib.h>
#include <string.h>

// ** app
#include <tiferet/domain/error.h>
#include <tiferet/utils/strutil.h>
```

### Code Snippets

Each logical step within a function is a separate snippet preceded by a comment:

```c
// Initialize the empty services map.
tif_map_init(&di->services);

// Resolve each configuration into a live service instance.
for (size_t i = 0; i < config_count; ++i) {
    TifService* svc = NULL;
    TifResult r = resolve_service(&configs[i], flags, flag_count, registry, &svc);

    // On failure, destroy already-resolved services and return the error.
    if (TIF_IS_ERR(r)) {
        tif_di_destroy(di);
        return r;
    }

    // Cache the resolved service by its configuration ID.
    tif_map_set(&di->services, configs[i].id, svc);
}
```

## Domain Objects

Defined in `include/tiferet/domain/` and `src/domain/`.

- **All `char*` fields are owned** — heap-allocated via `tif_strdup` in `_init`, freed in `_destroy`.
- **`_init`** copies all string inputs, initializes maps/arrays, derives missing fields.
- **`_destroy`** frees all owned fields and resets the struct to zero.
- **`_validate`** checks that required fields are non-empty; returns `bool`.
- **Arrays** (e.g., `steps`, `messages`) use `realloc`-grown heap arrays. `_destroy` iterates and destroys each element.

### Domain Modules

- `domain/error.h/.c` — `TifErrorMessage`, `TifError`
- `domain/feature.h/.c` — `TifFeatureEvent`, `TifFeature`
- `domain/app.h/.c` — `TifAppServiceDependency`, `TifAppInterface`
- `domain/di.h/.c` — `TifFlaggedDependency`, `TifServiceConfiguration`

## Events (Error Propagation)

Defined in `include/tiferet/events/settings.h` and `src/events/settings.c`.

```c
TIF_VERIFY(expr, error_code, message)  // early return if expr is false
TIF_VERIFY_CODE(expr, error_code)      // same, no message
TIF_RAISE(error_code, message)         // unconditional error return
TIF_RAISE_CODE(error_code)             // unconditional, no message
TIF_PROPAGATE(call)                    // propagate error from sub-call
```

Error codes are `#define` string constants in `include/tiferet/assets/constants.h`.

## Interfaces (Vtable Pattern)

Defined in `include/tiferet/interfaces/` (header-only — no `.c` files).

All interfaces embed `TifService base` as their **first member**. Concrete implementations embed the interface struct as their first member, enabling safe upcasting via pointer cast. All function pointers receive `self*` as the first argument.

- `interfaces/settings.h` — `TifService` base vtable
- `interfaces/feature.h` — `TifFeatureService` (get, exists)
- `interfaces/error.h` — `TifErrorService` (get, exists)
- `interfaces/container.h` — `TifContainerService`
- `interfaces/app.h` — `TifAppService`

## Mappers

Defined in `include/tiferet/mappers/` and `src/mappers/`.

In C, the mapper pattern uses **composition** instead of C++ multiple inheritance:

- **Aggregate** — a struct embedding the domain struct as `data`, plus mutation functions (`set_attribute`, `rename`, etc.).
- **TransferObject (YamlObject)** — a struct embedding the domain struct as `data`, plus serialization functions (`to_map`, `map`, `from_model`).

```c
// Aggregate
typedef struct { TifError data; } TifErrorAggregate;
TifResult tif_error_agg_rename(TifErrorAggregate* agg, const char* name);

// TransferObject
typedef struct { TifError data; } TifErrorYamlObject;
TifResult tif_error_yaml_map(const TifErrorYamlObject* obj, TifErrorAggregate* out);
```

- `mappers/error.h/.c` — `TifErrorAggregate`, `TifErrorYamlObject`
- `mappers/feature.h/.c` — `TifFeatureAggregate`, `TifFeatureYamlObject`
- `mappers/app.h/.c` — `TifAppInterfaceAggregate`, `TifAppInterfaceYamlObject`
- `mappers/di.h/.c` — `TifServiceConfigAggregate`, `TifServiceConfigYamlObject`

## Contexts

Defined in `include/tiferet/contexts/` and `src/contexts/`.

- **`TifServiceRegistry`** — maps `"module_path::class_name"` string keys to `TifFactoryFn` factory functions. Uses `TifFactoryWrapper` internally (avoids ISO C fn-ptr ↔ void* cast warnings).
- **`TifDIContainer`** — resolves `TifServiceConfiguration[]` into live `TifService*` via the registry. Owns all services; calls `destroy` on `tif_di_destroy`.
- **`TifExecutableEvent`** — vtable extending `TifService` with `execute(self, data, params, &result_out) → TifResult`. All feature step services must implement this.
- **`TifFeatureContext`** — iterates a `TifFeature`'s steps, resolves each from DI, invokes `execute`, stores results under `data_key`. Honors `pass_on_error`.
- **`TifErrorContext`** — thin wrapper over `TifErrorService` for `get` / `format` / `exists`.
- **`TifAppContext`** — top-level orchestrator combining feature service, error context, and feature context. Entry point: `tif_app_ctx_run`.

## Utilities

Defined in `include/tiferet/utils/` and `src/utils/`.

- **`TifMap`** (`utils/map.h/.c`) — open-addressing hash map. Keys are owned `char*`; values are caller-managed `void*`. FNV-1a hash, linear probing, tombstone removal.
- **`TifVector`** (`utils/vector.h/.c`) — dynamic array of `void*`. Push, get, remove, clear.
- **strutil** (`utils/strutil.h/.c`) — `tif_strdup`, `tif_str_to_upper`, `tif_str_to_lower`, `tif_str_to_snake`, `tif_str_to_upper_code`, `tif_str_make_key`, `tif_str_is_empty`.

## Build & Test

```bash
# Library + tests
cmake -S . -B build -DTIFERET_BUILD_TESTS=ON
make -C build
ctest --test-dir build

# Library only
cmake -S . -B build -DTIFERET_BUILD_TESTS=OFF
make -C build

# Library + examples
cmake -S . -B build -DTIFERET_BUILD_EXAMPLES=ON
make -C build
```

All code compiles with zero warnings under `-Wall -Wextra -Wpedantic` with AppleClang.

## Key Files for Orientation

- `include/tiferet/tiferet.h` — Umbrella header; start here
- `include/tiferet/assets/errors.h` — `TifResult` type and macros
- `include/tiferet/assets/constants.h` — All `TIF_ERR_*` error code constants
- `include/tiferet/events/settings.h` — `TIF_VERIFY`, `TIF_PROPAGATE`, `TIF_RAISE`
- `include/tiferet/interfaces/settings.h` — `TifService` base vtable
- `include/tiferet/contexts/registry.h` — `TifServiceRegistry` + `TifFactoryFn`
- `include/tiferet/contexts/di.h` — `TifDIContainer`
- `include/tiferet/contexts/feature.h` — `TifExecutableEvent` + `TifFeatureContext`
- `include/tiferet/contexts/app.h` — `TifAppContext` (top-level entry point)
- `examples/calculator/main.c` — End-to-end working example
- `CMakeLists.txt` — Build configuration
- `handoff.md` — Phase-by-phase development history and design decisions

## Contributing

See `CONTRIBUTING.md` for the workflow. For TRD templates, code style guides, and collaboration report format, refer to the canonical [Tiferet framework documentation](https://github.com/greatstrength/tiferet/tree/main/docs).
