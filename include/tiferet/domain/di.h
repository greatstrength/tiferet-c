// *** tiferet/domain/di.h
// Dependency injection domain models for the Tiferet C framework.
// TifFlaggedDependency defines a flag-specific implementation override.
// TifServiceConfiguration defines the DI wiring for a service.
// Mirrors C++ tiferet/domain/di.h, C# ServiceConfiguration,
// and Python tiferet/domain/di.py.

#ifndef TIFERET_DOMAIN_DI_H
#define TIFERET_DOMAIN_DI_H

#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

#include <stdbool.h>
#include <stddef.h>

// *** models

// ** model: tif_flagged_dependency
/// A flag-specific implementation override for a service configuration.
typedef struct {
    char* module_path;  // Owned. Module path for the dependency class.
    char* class_name;   // Owned. Class name for the dependency.
    char* flag;         // Owned. The flag that activates this dependency.
    TifMap parameters;  // Owned. Configuration parameters.
} TifFlaggedDependency;

// ** model: tif_service_configuration
/// A service configuration that defines dependency injection behavior.
typedef struct {
    char* id;                           // Owned. Unique identifier.
    char* name;                         // Owned. Optional display name.
    char* module_path;                  // Owned. Default module path.
    char* class_name;                   // Owned. Default class name.
    TifMap parameters;                  // Owned. Default parameters.
    TifFlaggedDependency* dependencies; // Owned array of flag overrides.
    size_t dependency_count;            // Number of flag overrides.
} TifServiceConfiguration;

// *** functions: TifFlaggedDependency

/// Initialize a flagged dependency. Copies all string inputs.
TifResult tif_flagged_dep_init(TifFlaggedDependency* dep,
                               const char* module_path,
                               const char* class_name,
                               const char* flag);

/// Destroy a flagged dependency, freeing all owned fields.
void tif_flagged_dep_destroy(TifFlaggedDependency* dep);

/// Validate a flagged dependency.
bool tif_flagged_dep_validate(const TifFlaggedDependency* dep);

// *** functions: TifServiceConfiguration

/// Initialize a service configuration. Copies all string inputs.
TifResult tif_service_config_init(TifServiceConfiguration* cfg,
                                  const char* id,
                                  const char* name,
                                  const char* module_path,
                                  const char* class_name);

/// Destroy a service configuration, freeing all owned fields.
void tif_service_config_destroy(TifServiceConfiguration* cfg);

/// Validate a service configuration. id must be non-empty.
bool tif_service_config_validate(const TifServiceConfiguration* cfg);

/// Add a flagged dependency. The dependency is moved (shallow copy).
TifResult tif_service_config_add_dependency(TifServiceConfiguration* cfg,
                                            TifFlaggedDependency dep);

/// Find the first flagged dependency matching any of the provided flags.
/// Flags are checked in order (priority). Returns NULL if none match.
const TifFlaggedDependency* tif_service_config_get_dependency(
    const TifServiceConfiguration* cfg,
    const char** flags,
    size_t flag_count);

#endif // TIFERET_DOMAIN_DI_H
