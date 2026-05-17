# Tiferet-C Handoff Document

**Branch:** `v1.0b1-release`  
**Last Tag:** `v1.0.0a4`  
**Date:** May 17, 2026  

## What This Project Is

Tiferet-C is a pure C implementation of the Tiferet framework — a DDD (Domain-Driven Design) framework that exists in four languages: Python (`tiferet`), C# (`tiferet.net`), C++ (`tiferet-cpp`), and now C (`tiferet-c`). The goal is a unified application modeling language across all four, where each adapts to its language's idioms while maintaining consistent architecture.

## What Has Been Completed (Phases 1–4)

### Phase 1: Foundation (`v1.0.0a1`)
- **`CMakeLists.txt`** — C11 static library, `-Wall -Wextra -Wpedantic`, optional `TIFERET_YAML` for libyaml
- **`include/tiferet/assets/constants.h`** — Error code string constants (`TIF_ERR_*` macros)
- **`include/tiferet/assets/errors.h`** + `src/assets/errors.c` — `TifResult` error propagation type with `TIF_OK`, `TIF_ERR`, `TIF_ERR_MSG`, `TIF_IS_OK`, `TIF_IS_ERR` macros and `tif_result_format()`
- **`include/tiferet/utils/strutil.h`** + `src/utils/strutil.c` — `tif_strdup`, `tif_str_to_upper`, `tif_str_to_lower`, `tif_str_to_snake`, `tif_str_to_upper_code`, `tif_str_make_key`, `tif_str_is_empty`
- **`include/tiferet/utils/vector.h`** + `src/utils/vector.c` — `TifVector` dynamic array (push, get, remove, clear)
- **`include/tiferet/utils/map.h`** + `src/utils/map.c` — `TifMap` open-addressing hash map with FNV-1a, tombstones, iterator

### Phase 2: Domain Objects (`v1.0.0a2`)
- **`domain/settings.h`** — Documentation-only header defining the `_init`/`_destroy`/`_validate` convention
- **`domain/error.h/.c`** — `TifErrorMessage`, `TifError` with `derive_error_code`, `format_message`, `add_message`
- **`domain/feature.h/.c`** — `TifFeatureEvent`, `TifFeature` with `derive_keys`, `add_step`, `get_step`
- **`domain/app.h/.c`** — `TifAppServiceDependency`, `TifAppInterface` with `add_flag`, `add_service`, `get_service`
- **`domain/di.h/.c`** — `TifFlaggedDependency`, `TifServiceConfiguration` with `add_dependency`, `get_dependency`

### Phase 3: Events + Interfaces (`v1.0.0a3`)
- **`events/settings.h/.c`** — `TIF_VERIFY`, `TIF_VERIFY_CODE`, `TIF_RAISE`, `TIF_RAISE_CODE`, `TIF_PROPAGATE` macros; `tif_raise_error()` function
- **`interfaces/settings.h`** — `TifService` base vtable (header-only)
- **`interfaces/feature.h`** — `TifFeatureService` vtable (header-only)
- **`interfaces/error.h`** — `TifErrorService` vtable (header-only)
- **`interfaces/container.h`** — `TifContainerService` vtable (header-only)
- **`interfaces/app.h`** — `TifAppService` vtable (header-only)

### Phase 4: Mappers (`v1.0.0a4`)
- **`mappers/settings.h`** — Documentation-only header for aggregate/transfer object conventions
- **`mappers/error.h/.c`** — `TifErrorAggregate` (set_attribute, rename, set_message, remove_message), `TifErrorYamlObject` (to_map, map, from_model)
- **`mappers/feature.h/.c`** — `TifFeatureEventAggregate`, `TifFeatureAggregate`, and their YamlObject counterparts
- **`mappers/app.h/.c`** — `TifAppInterfaceAggregate`, `TifAppInterfaceYamlObject`
- **`mappers/di.h/.c`** — `TifServiceConfigAggregate`, `TifServiceConfigYamlObject`

## What Remains (Phases 5–6)

### Phase 5: Runtime Contexts (`v1.0.0a5`)
Refer to the plan (created earlier in the conversation) for detailed design. Key files to create:

- **`contexts/registry.h/.c`** — `TifServiceRegistry` mapping `"module_path::class_name"` keys to `TifFactoryFn` factory functions. Use `tif_str_make_key()` from strutil. Methods: `init`, `destroy`, `register`, `create`, `has`, `size`.
- **`contexts/di.h/.c`** — `TifDIContainer` that resolves `TifServiceConfiguration` entries into live `TifService*` using the registry. Checks flagged dependencies first (priority order), falls back to default module_path/class_name.
- **`contexts/feature.h/.c`** — `TifExecutableEvent` vtable (extends `TifService` with an `execute` function pointer taking `TifMap* data` + `TifMap* params`). `TifFeatureContext` that iterates a feature's steps, resolves each from the DI container, casts to `TifExecutableEvent*`, and invokes. Honors `pass_on_error` and `data_key`.
- **`contexts/error.h/.c`** — `TifErrorContext` wrapping a `TifErrorService*` for error formatting.
- **`contexts/app.h/.c`** — `TifAppContext` top-level orchestrator: holds feature service, error service, DI container, feature context. `tif_app_run(app, feature_id, data)` loads a feature and executes it.

Mirror the C++ implementations in `tiferet-cpp/include/tiferet/contexts/` for exact behavior.

### Phase 6: Calculator Example + Umbrella Header (`v1.0.0a6`)
- **`include/tiferet/tiferet.h`** — Umbrella header including all public headers (mirror C++ `tiferet.h`)
- **`examples/calculator/main.c`** — Calculator demo using `AddEvent`, `SubtractEvent`, `MultiplyEvent`, `DivideEvent` as `TifExecutableEvent` implementations. Registers them in a `TifServiceRegistry`, builds a `TifDIContainer` from service configs, creates `TifAppContext`, runs test cases.
- **`examples/calculator/configs/`** — YAML config files (reuse from C++ `examples/calculator/configs/`)

## Key Design Decisions

### File Placement Rules
- **`include/`** = Shape (struct layout), Contract (function signature), Value (constant/macro) → "What is this and what can it do?"
- **`src/`** = Behavior (function body) → "How does it do it?"
- **Interfaces are header-only** — pure function pointer contracts, no `.c` file
- **Assets constants are header-only** — `#define` macros, no `.c` file

### Naming Convention
- Types: `Tif` PascalCase prefix (`TifError`, `TifMap`)
- Functions: `tif_` snake_case (`tif_error_init`, `tif_map_set`)
- Constants: `TIF_` SCREAMING_SNAKE (`TIF_ERR_FEATURE_NOT_FOUND`)
- Macros: `TIF_` SCREAMING_SNAKE (`TIF_VERIFY`, `TIF_PROPAGATE`)

### Error Propagation
C has no exceptions. All fallible functions return `TifResult`. Use macros:
- `TIF_VERIFY(expr, code, msg)` — early-return on failure
- `TIF_PROPAGATE(call)` — chain fallible sub-calls
- `TIF_RAISE(code, msg)` — unconditional error return

### Ownership
All `char*` fields in structs are **owned** (heap-allocated via `tif_strdup`). `_init` copies inputs, `_destroy` frees them. `TifMap` owns its keys but values are caller-managed `void*`.

### Mappers
C uses **composition** (embedded domain object in a wrapper struct) instead of C++ multiple inheritance. Aggregate = mutation functions, YamlObject = serialization functions.

## Reference Projects
- **C++**: `/Users/ashatz/Documents/GitHub/tiferet-cpp/` — closest structural reference
- **C#**: `/Users/ashatz/Documents/GitHub/tiferet.net/Tiferet/` — for domain/event/mapper patterns
- **Python**: `/Users/ashatz/Documents/GitHub/tiferet/` (on branch `v1.x-proto`) — canonical framework

## Build
```bash
cmake -S . -B build -G "Unix Makefiles" -DTIFERET_BUILD_TESTS=OFF -DTIFERET_BUILD_EXAMPLES=OFF
make -C build
```

All phases compile with zero warnings under `-Wall -Wextra -Wpedantic` with AppleClang.
