// *** tiferet/interfaces/container.h
// Service interface for the DI container.
// Mirrors Python ContainerService, C# IDIService,
// and C++ DIContainer (as a service contract).

#ifndef TIFERET_INTERFACES_CONTAINER_H
#define TIFERET_INTERFACES_CONTAINER_H

#include <tiferet/interfaces/settings.h>
#include <tiferet/domain/di.h>

#include <stdbool.h>
#include <stddef.h>

// *** interfaces

// ** interface: tif_container_service
/// Abstract service contract for the dependency injection container.
/// Provides retrieval of service configurations and resolved services.
typedef struct TifContainerService {

    /// Base service vtable. Must be the first member.
    TifService base;

    /// Retrieve a resolved service by its configuration ID.
    /// Returns a pointer to the service, or NULL if not found.
    /// The returned pointer is owned by the container — do not free it.
    TifService* (*get_service)(struct TifContainerService* self,
                               const char* id);

    /// Check whether a service has been resolved for the given ID.
    bool (*has_service)(struct TifContainerService* self, const char* id);

    /// Return the number of resolved services.
    size_t (*service_count)(struct TifContainerService* self);

} TifContainerService;

#endif // TIFERET_INTERFACES_CONTAINER_H
