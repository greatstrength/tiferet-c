// *** tiferet/contexts/di.h
// DI container for the Tiferet C framework.
// Resolves TifServiceConfiguration objects into live TifService instances
// using the TifServiceRegistry. Mirrors C++ DIContainer and Python DIContext.
//
// For each configuration, the container checks flagged dependencies first
// (in flag priority order), falls back to the default module_path/class_name,
// builds the registry key, creates the service, and caches it.

#ifndef TIFERET_CONTEXTS_DI_H
#define TIFERET_CONTEXTS_DI_H

#include <tiferet/contexts/registry.h>
#include <tiferet/domain/di.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

#include <stdbool.h>
#include <stddef.h>

// *** contexts

// ** context: tif_di_container
/// A dependency injection container that resolves TifServiceConfiguration
/// entries into concrete TifService instances using a TifServiceRegistry.
/// Mirrors C++ DIContainer.
///
/// Usage:
///   TifDIContainer di;
///   tif_di_init(&di, configs, count, flags, flag_count, &registry);
///   TifService* svc = tif_di_get(&di, "my_service_id");
///   // ... use svc ...
///   tif_di_destroy(&di);  // calls destroy on all owned services
typedef struct {
    TifMap services;    // Owned. Maps config.id -> TifService* (owned).
} TifDIContainer;

// *** functions

/// Initialize the DI container by resolving all service configurations.
/// For each config, checks flagged dependencies first (in flags order),
/// then falls back to the default module_path/class_name.
/// Builds the registry key and creates the service via the registry.
///
/// On failure, any already-resolved services are destroyed before returning.
///
/// :param di:           The container to initialize.
/// :param configs:      Array of service configurations to resolve.
/// :param config_count: Number of configurations.
/// :param flags:        Feature flags for dependency selection (priority order).
/// :param flag_count:   Number of flags.
/// :param registry:     Registry containing the factory functions.
TifResult tif_di_init(TifDIContainer* di,
                      const TifServiceConfiguration* configs,
                      size_t config_count,
                      const char** flags,
                      size_t flag_count,
                      const TifServiceRegistry* registry);

/// Destroy the DI container.
/// Calls destroy on every owned TifService, then frees the internal map.
void tif_di_destroy(TifDIContainer* di);

/// Retrieve a resolved service by its configuration ID.
/// Returns NULL if the ID was not resolved.
/// The returned pointer is owned by the container — do not free it.
TifService* tif_di_get(const TifDIContainer* di, const char* id);

/// Check whether a service has been resolved for the given ID.
bool tif_di_has(const TifDIContainer* di, const char* id);

/// Return the number of resolved services.
size_t tif_di_size(const TifDIContainer* di);

#endif // TIFERET_CONTEXTS_DI_H
