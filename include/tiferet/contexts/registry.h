// *** tiferet/contexts/registry.h
// Service registry for the Tiferet C framework.
// Maps "module_path::class_name" keys to TifFactoryFn factory functions
// that produce concrete TifService instances.
// Mirrors C++ ServiceRegistry and Python's dynamic import pattern.

#ifndef TIFERET_CONTEXTS_REGISTRY_H
#define TIFERET_CONTEXTS_REGISTRY_H

#include <tiferet/interfaces/settings.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

#include <stdbool.h>
#include <stddef.h>

// *** types

// ** type: tif_factory_fn
/// Factory function signature: receives a parameter map, returns an owning
/// TifService pointer. Returns NULL on allocation failure.
/// The parameter map values are const char* strings (caller-managed).
typedef TifService* (*TifFactoryFn)(const TifMap* params);

// *** contexts

// ** context: tif_service_registry
/// A registry that maps "module_path::class_name" string keys to
/// TifFactoryFn factory functions for creating TifService instances.
/// Mirrors C++ ServiceRegistry.
///
/// Keys follow the format "module_path::class_name" to mirror the
/// module_path/class_name fields in TifServiceConfiguration.
///
/// Usage:
///   TifServiceRegistry reg;
///   tif_registry_init(&reg);
///   tif_registry_register(&reg, "calc.events::AddNumber", add_number_factory);
///   TifService* svc = NULL;
///   tif_registry_create(&reg, "calc.events::AddNumber", NULL, &svc);
///   // ... use svc ...
///   svc->destroy(svc);
///   tif_registry_destroy(&reg);
typedef struct {
    TifMap factories;   // Owned. Maps key -> TifFactoryWrapper* (internal).
} TifServiceRegistry;

// *** functions

/// Initialize an empty service registry.
void tif_registry_init(TifServiceRegistry* reg);

/// Destroy the registry, freeing all owned keys and factory wrappers.
/// Does NOT destroy any TifService instances created by the registry.
void tif_registry_destroy(TifServiceRegistry* reg);

/// Register a factory function under the given key.
/// Overwrites any existing registration for the same key.
/// Returns false on allocation failure.
bool tif_registry_register(TifServiceRegistry* reg,
                           const char* key,
                           TifFactoryFn factory);

/// Create a TifService instance by key, passing params to the factory.
/// Sets *out to the created service on success.
/// Returns TIF_ERR_SERVICE_NOT_REGISTERED if the key is not found.
/// Returns TIF_ERR_SERVICE_RESOLUTION_FAILED if the factory returns NULL.
TifResult tif_registry_create(const TifServiceRegistry* reg,
                              const char* key,
                              const TifMap* params,
                              TifService** out);

/// Check whether a key is registered.
bool tif_registry_has(const TifServiceRegistry* reg, const char* key);

/// Return the number of registered factories.
size_t tif_registry_size(const TifServiceRegistry* reg);

#endif // TIFERET_CONTEXTS_REGISTRY_H
