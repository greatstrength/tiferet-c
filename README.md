# tiferet-c — Domain-Driven Design in Pure C

[![License](https://img.shields.io/github/license/greatstrength/tiferet-c?style=flat-square)](LICENSE)
[![Language](https://img.shields.io/badge/language-C11-blue?style=flat-square)](https://en.cppreference.com/w/c/11)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey?style=flat-square)]()

**tiferet-c** is a pure C11 implementation of the [Tiferet](https://github.com/greatstrength/tiferet) framework — a Domain-Driven Design (DDD) framework available across four languages (Python, C#, C++, and C). It brings structured, configuration-driven application modeling to systems-level C with zero mandatory dependencies.

Inspired by the Kabbalistic principle of beauty in balance, Tiferet turns domain complexity into clean, layered, testable C code — from domain structs through a full runtime pipeline.

### At a glance
- Pure C11 — no mandatory external dependencies
- Domain objects as plain structs with `_init` / `_destroy` / `_validate` conventions
- Vtable-based service interfaces and domain event execution
- `TifResult` error propagation — no setjmp, no exceptions
- Structured configuration-driven feature workflows via `TifFeature` / `TifDIContainer`
- Full runtime pipeline: registry → DI container → feature context → app context
- Single umbrella header: `#include <tiferet/tiferet.h>`

**Current status:** `v1.0.0b1` (beta — all six phases complete, actively hardening toward stable v1)

## Quick Start

### Build

```bash
git clone https://github.com/greatstrength/tiferet-c
cmake -S tiferet-c -B tiferet-c/build -DTIFERET_BUILD_EXAMPLES=ON
make -C tiferet-c/build
```

### Run the calculator example

```bash
./tiferet-c/build/examples/calculator/calculator
```

Expected output:

```
1 + 2 = 3
5 - 3 = 2
4 * 3 = 12
8 / 2 = 4
Error: Cannot divide by zero
```

The calculator wires four arithmetic events through the full Tiferet pipeline — registry, DI container, feature context, and app context — entirely in C. See [`examples/calculator/main.c`](examples/calculator/main.c).

## Architecture

```
tiferet-c/
├── include/tiferet/
│   ├── tiferet.h          # Umbrella header (include this)
│   ├── assets/            # Error codes (TIF_ERR_*) and TifResult type
│   ├── domain/            # Domain structs: TifError, TifFeature, TifAppInterface, TifServiceConfiguration
│   ├── events/            # TIF_VERIFY / TIF_RAISE / TIF_PROPAGATE macros
│   ├── interfaces/        # TifService vtable and domain service interfaces
│   ├── mappers/           # Aggregates (mutation) and transfer objects (serialization)
│   ├── contexts/          # TifServiceRegistry, TifDIContainer, TifFeatureContext, TifAppContext
│   └── utils/             # TifMap, TifVector, string utilities
└── src/                   # Implementations (mirrors include/ structure)
```

### Runtime flow

1. Register factory functions in a `TifServiceRegistry` (`"module::Class"` → `TifFactoryFn`)
2. Build a `TifDIContainer` from `TifServiceConfiguration` entries + registry
3. Create a `TifAppContext` with feature service, error service, and DI container
4. Call `tif_app_ctx_run(&app, "feature.id", &data)` — looks up the feature and executes its step pipeline
5. Each step casts to `TifExecutableEvent*`, calls `execute(data, params, &result)`, stores result under `data_key`

### Error propagation

C has no exceptions. All fallible functions return `TifResult`:

```c
TifResult r = tif_feature_init(&feat, "calc.add", "Add", "calc", "add", NULL);
if (TIF_IS_ERR(r)) { /* handle */ }

// Or chain with macros:
TIF_PROPAGATE(tif_feature_init(&feat, "calc.add", "Add", "calc", "add", NULL));
TIF_VERIFY(b != 0.0, TIF_ERR_DIVISION_BY_ZERO, "Cannot divide by zero");
```

## Documentation

**Core architecture**
- [Code Style & Artifact Comments](docs/core/code_style.md)
- [Domain Objects](docs/core/domain.md)
- [Events & Error Propagation](docs/core/events.md)
- [Service Interfaces (Vtable Pattern)](docs/core/interfaces.md)
- [Mappers (Aggregates & Transfer Objects)](docs/core/mappers.md)
- [Runtime Contexts](docs/core/contexts.md)
- [Utilities (TifMap, TifVector, strutil)](docs/core/utils.md)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow.

For TRD templates, code style guides, and collaboration report format, refer to the canonical [Tiferet framework documentation](https://github.com/greatstrength/tiferet/tree/main/docs).

Questions or feedback? Open an issue or reach out: andrew@greatstrength.me
