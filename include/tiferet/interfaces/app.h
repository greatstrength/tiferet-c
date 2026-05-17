// *** tiferet/interfaces/app.h
// Service interface for app interface retrieval.
// Mirrors Python AppService, C# IAppService,
// and C++ AppService.

#ifndef TIFERET_INTERFACES_APP_H
#define TIFERET_INTERFACES_APP_H

#include <tiferet/interfaces/settings.h>
#include <tiferet/domain/app.h>

#include <stdbool.h>

// *** interfaces

// ** interface: tif_app_service
/// Abstract service contract for retrieving AppInterface definitions.
/// Consumed by AppContext to load interface configurations.
typedef struct TifAppService {

    /// Base service vtable. Must be the first member.
    TifService base;

    /// Retrieve an app interface by its identifier.
    /// Returns a pointer to the interface, or NULL if not found.
    /// The returned pointer is owned by the service — do not free it.
    const TifAppInterface* (*get)(struct TifAppService* self, const char* id);

    /// Check whether an app interface exists by its identifier.
    bool (*exists)(struct TifAppService* self, const char* id);

} TifAppService;

#endif // TIFERET_INTERFACES_APP_H
