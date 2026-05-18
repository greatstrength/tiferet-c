# Contributing to tiferet-c

Thank you for your interest in contributing to the Tiferet C framework. This document outlines the workflow for contributions to this repository.

## Getting Started

1. **Fork** the repository and clone your fork locally.
2. Create a feature branch from the current release branch (e.g., `v1.0b1-release`):
   ```bash
   git checkout -b <issue-number>-<short-description> v1.0b1-release
   ```
3. Build and verify the baseline compiles with zero warnings:
   ```bash
   cmake -S . -B build -DTIFERET_BUILD_TESTS=ON
   make -C build
   ```

## Contribution Workflow

### 1. Open or Claim an Issue

All contributions must be tied to a GitHub issue. If one doesn't exist for the work you'd like to do, open an issue first.

### 2. Write a Technical Requirements Document (TRD)

For non-trivial changes, a TRD is required before implementation. Use the TRD template and authoring guide from the canonical Tiferet framework documentation:

**→ [TRD guide and template](https://github.com/greatstrength/tiferet/blob/main/docs/collab/tech_requirements.md)**

### 3. Implement

Follow the structured code style consistently. Refer to the guides in `docs/core/` for this repository, and the canonical style and component guides from the Tiferet framework:

**→ [Code style & component guides](https://github.com/greatstrength/tiferet/tree/main/docs/core)**

Key rules for this C repository:
- All code must compile with **zero warnings** under `-Wall -Wextra -Wpedantic`.
- All public headers go in `include/tiferet/<layer>/`; implementations in `src/<layer>/`.
- All `char*` fields in domain structs are **owned** — heap-allocated in `_init`, freed in `_destroy`.
- Use `TIF_VERIFY`, `TIF_RAISE`, and `TIF_PROPAGATE` (never raw `return` for errors).
- Follow artifact comment conventions from [`docs/core/code_style.md`](docs/core/code_style.md).

### 4. Commit Hygiene

- Separate functional changes from documentation in distinct commits.
- Title commits by scope (e.g., `Domain – TifError init/destroy`, `Docs – update code_style`).
- Include `Co-Authored-By: Oz <oz-agent@warp.dev>` when collaborating with an AI agent.

### 5. Open a Pull Request

- Target the current **release branch** (e.g., `v1.0b1-release`), not `main`.
- Reference the GitHub issue in the PR description.
- Ensure all tests pass and the build is warning-free.

### 6. Collaboration Report

Upon completion, publish a Collaboration Report as a comment on the originating issue. Use the report format from the canonical Tiferet documentation:

**→ [Collaboration report format](https://github.com/greatstrength/tiferet/blob/main/docs/collab/collab_report.md)**

## Branch Naming

Feature branches follow the pattern: `<issue-number>-<lowercase-hyphenated-title>`

Examples:
- `42-domain-tif-error` 
- `57-contexts-di-container`

## Code Style Reference

- **This repository:** [`docs/core/code_style.md`](docs/core/code_style.md)
- **Canonical framework guides:** https://github.com/greatstrength/tiferet/tree/main/docs/core

## License

By contributing to tiferet-c, you agree that your contributions will be licensed under the [BSD 3-Clause License](LICENSE) that covers the project.
