# Structured Code Style in tiferet-c

**Project:** tiferet-c  
**Repository:** https://github.com/greatstrength/tiferet-c

## Overview

tiferet-c enforces a structured code style to ensure consistency, readability, extensibility, and AI-parsability across all components. The style relies on **artifact comments** for hierarchical organisation and strict conventions for naming, spacing, and code snippets.

This document defines the required code style for all files in the `src/` and `include/` trees.

## File Placement

Two rules govern where code lives:

- **`include/tiferet/<layer>/`** — *Shape, contract, value*: struct layouts, function declarations, `#define` macros. Everything a consumer needs to call the API.
- **`src/<layer>/`** — *Behaviour*: function bodies only. No type definitions or declarations here.
- **Interfaces** (`interfaces/`) — header-only. Pure vtable structs; no `.c` file.
- **Assets constants** (`assets/constants.h`) — header-only. `#define` macros; no `.c` file.

## Artifact Comments: Hierarchy and Purpose

Artifact comments provide a machine-readable structure that organises code into predictable sections.

### Top-Level (`// ***`)

Denotes major file sections:

```c
// *** includes      — all #include directives
// *** constants     — #define constants
// *** types         — typedef declarations
// *** models        — domain struct definitions
// *** events        — event macros / functions
// *** interfaces    — vtable struct definitions
// *** mappers       — aggregate / transfer object definitions
// *** contexts      — context struct definitions
// *** utils         — utility struct definitions
// *** functions     — standalone function implementations
// *** internal functions  — file-private (static) helpers
```

**Spacing:** one empty line between the top-level comment and the first mid-level comment.

### Mid-Level (`// **`)

Identifies a category or individual component within a section:

```c
// ** core           — standard library includes
// ** app            — project includes
// ** model: tif_error
// ** function: tif_error_init
// ** util: tif_map
// ** context: tif_di_container
```

**Spacing:** one empty line between each mid-level block.

### Low-Level (`// *`)

Names sub-components within a struct or function group:

```c
// * attribute: id
// * init
// * method: format_message
// * method: derive_error_code (static)
// * function: tif_error_init
```

**Spacing:** one empty line between low-level sections and their code blocks.

## Naming Conventions

| Kind | Convention | Example |
|---|---|---|
| Types / structs | `Tif` + PascalCase | `TifError`, `TifMap`, `TifDIContainer` |
| Functions | `tif_` + snake_case | `tif_error_init`, `tif_map_set` |
| Error code macros | `TIF_ERR_` + SCREAMING_SNAKE | `TIF_ERR_FEATURE_NOT_FOUND` |
| General macros | `TIF_` + SCREAMING_SNAKE | `TIF_VERIFY`, `TIF_OK`, `TIF_IS_ERR` |
| File-private helpers | `static` + snake_case | `static size_t hash_string(...)` |

## Include Organisation

```c
// *** includes

// ** core
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ** app
#include <tiferet/domain/error.h>
#include <tiferet/utils/strutil.h>
```

Standard library headers go under `// ** core`; project headers under `// ** app`.

## Code Snippets

Each logical step within a function body is a separate snippet preceded by a descriptive comment. One empty line separates snippets.

```c
// ** function: tif_error_init
TifResult tif_error_init(TifError* err,
                         const char* id,
                         const char* name,
                         const char* description,
                         const char* error_code) {

    // Zero-initialize the struct.
    memset(err, 0, sizeof(TifError));

    // Copy id.
    err->id = tif_strdup(id);
    if (id && !err->id) return TIF_ERR("ALLOC_FAILED");

    // Copy name.
    err->name = tif_strdup(name);
    if (name && !err->name) {
        tif_error_destroy(err);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Derive or copy error_code.
    if (error_code) {
        err->error_code = tif_strdup(error_code);
    } else if (id) {
        err->error_code = tif_error_derive_error_code(id);
    }

    // Return success.
    return TIF_OK;
}
```

## Spacing Rules

- One empty line between `// ***` and the first `// **`.
- One empty line between `// **` blocks.
- One empty line between `// *` sections and their code.
- One empty line between code snippets within a function.
- One empty line between function definitions in a `.c` file.

## Function Parameter Alignment

For functions with more than three parameters, align parameters with the opening parenthesis:

```c
TifResult tif_feature_init(TifFeature* feat,
                           const char* id,
                           const char* name,
                           const char* group_id,
                           const char* feature_key,
                           const char* description);
```

## Complete Example

**Header** (`include/tiferet/domain/error.h`):

```c
// *** tiferet/domain/error.h
// Error domain models for the Tiferet C framework.

#ifndef TIFERET_DOMAIN_ERROR_H
#define TIFERET_DOMAIN_ERROR_H

#include <tiferet/assets/errors.h>
#include <stdbool.h>
#include <stddef.h>

// *** models

// ** model: tif_error_message
/// A localized error message.
typedef struct {
    char* lang;     // Owned.
    char* text;     // Owned.
} TifErrorMessage;

// ** model: tif_error
/// An error definition with identifier, derived error code, and messages.
typedef struct {
    char* id;
    char* name;
    char* error_code;
    TifErrorMessage* messages;
    size_t message_count;
} TifError;

// *** functions: TifError

TifResult tif_error_init(TifError* err, const char* id, const char* name,
                         const char* description, const char* error_code);
void      tif_error_destroy(TifError* err);
bool      tif_error_validate(const TifError* err);

#endif // TIFERET_DOMAIN_ERROR_H
```

**Source** (`src/domain/error.c`):

```c
// *** src/domain/error.c

#include <tiferet/domain/error.h>
#include <tiferet/utils/strutil.h>
#include <stdlib.h>
#include <string.h>

// *** functions: TifError

// ** function: tif_error_init
TifResult tif_error_init(TifError* err, const char* id, const char* name,
                         const char* description, const char* error_code) {

    // Zero-initialize the struct.
    memset(err, 0, sizeof(TifError));

    // Copy id and name.
    err->id   = tif_strdup(id);
    err->name = tif_strdup(name);

    // Derive error_code from id if not provided.
    err->error_code = error_code
        ? tif_strdup(error_code)
        : tif_error_derive_error_code(id);

    return TIF_OK;
}
```

## Best Practices Summary

- Use artifact comments at every level — top, mid, and low.
- Name `static` helpers descriptively; mark them with `// * method: <name> (static)`.
- Keep functions focused on one operation.
- Break every logical step into a commented snippet.
- Never use raw `return` for errors — use `TIF_RAISE`, `TIF_VERIFY`, or `TIF_PROPAGATE`.
- Zero-initialize structs at the start of `_init` functions with `memset`.

## Related Guides

- [Domain Objects](domain.md)
- [Events & Error Propagation](events.md)
- [Service Interfaces](interfaces.md)
- [Mappers](mappers.md)
- [Runtime Contexts](contexts.md)
- [Utilities](utils.md)
