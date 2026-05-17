// *** tiferet/domain/app.h
// App domain models for the Tiferet C framework.
// TifAppServiceDependency defines a service binding for an app interface.
// TifAppInterface defines the application interface configuration.
// Mirrors C++ tiferet/domain/app.h, C# AppInterfaceConfiguration,
// and Python tiferet/domain/app.py.

#ifndef TIFERET_DOMAIN_APP_H
#define TIFERET_DOMAIN_APP_H

#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

#include <stdbool.h>
#include <stddef.h>

// *** models

// ** model: tif_app_service_dependency
/// A service dependency that defines a class binding for an app interface.
typedef struct {
    char* service_id;       // Owned. Service identifier.
    char* module_path;      // Owned. Module path for the dependency class.
    char* class_name;       // Owned. Class name for the dependency.
    TifMap parameters;      // Owned. Configuration parameters.
} TifAppServiceDependency;

// ** model: tif_app_interface
/// The application interface configuration.
typedef struct {
    char* id;                           // Owned. Unique identifier.
    char* name;                         // Owned. Display name.
    char* description;                  // Owned. Optional.
    char* module_path;                  // Owned. Module path for the context class.
    char* class_name;                   // Owned. Class name for the context.
    char* logger_id;                    // Owned. Logger identifier (default: "default").
    char** flags;                       // Owned array of flag strings.
    size_t flag_count;                  // Number of flags.
    TifAppServiceDependency* services;  // Owned array of service dependencies.
    size_t service_count;               // Number of services.
    TifMap constants;                   // Owned. Application constants.
} TifAppInterface;

// *** functions: TifAppServiceDependency

/// Initialize a service dependency. Copies all string inputs.
TifResult tif_app_service_dep_init(TifAppServiceDependency* dep,
                                   const char* service_id,
                                   const char* module_path,
                                   const char* class_name);

/// Destroy a service dependency, freeing all owned fields.
void tif_app_service_dep_destroy(TifAppServiceDependency* dep);

/// Validate a service dependency.
bool tif_app_service_dep_validate(const TifAppServiceDependency* dep);

// *** functions: TifAppInterface

/// Initialize an app interface. Copies all string inputs.
/// Flags default to {"default"} if none provided.
TifResult tif_app_interface_init(TifAppInterface* iface,
                                 const char* id,
                                 const char* name,
                                 const char* module_path,
                                 const char* class_name,
                                 const char* description,
                                 const char* logger_id);

/// Destroy an app interface, freeing all owned fields.
void tif_app_interface_destroy(TifAppInterface* iface);

/// Validate an app interface. id and name must be non-empty.
bool tif_app_interface_validate(const TifAppInterface* iface);

/// Add a flag to the interface.
TifResult tif_app_interface_add_flag(TifAppInterface* iface, const char* flag);

/// Add a service dependency to the interface.
/// The dependency is moved (shallow copy).
TifResult tif_app_interface_add_service(TifAppInterface* iface,
                                        TifAppServiceDependency dep);

/// Find a service dependency by service_id, or NULL if not found.
const TifAppServiceDependency* tif_app_interface_get_service(
    const TifAppInterface* iface, const char* service_id);

#endif // TIFERET_DOMAIN_APP_H
