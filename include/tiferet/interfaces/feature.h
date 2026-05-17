// *** tiferet/interfaces/feature.h
// Service interface for feature retrieval.
// Mirrors Python FeatureService, C# IFeatureService,
// and C++ FeatureService.

#ifndef TIFERET_INTERFACES_FEATURE_H
#define TIFERET_INTERFACES_FEATURE_H

#include <tiferet/interfaces/settings.h>
#include <tiferet/domain/feature.h>

#include <stdbool.h>

// *** interfaces

// ** interface: tif_feature_service
/// Abstract service contract for retrieving Feature definitions.
/// Consumed by AppContext and FeatureContext to load features
/// from persistent configuration (e.g., YAML).
typedef struct TifFeatureService {

    /// Base service vtable. Must be the first member.
    TifService base;

    /// Retrieve a feature by its composite identifier (group_id.feature_key).
    /// Returns a pointer to the feature, or NULL if not found.
    /// The returned pointer is owned by the service — do not free it.
    const TifFeature* (*get)(struct TifFeatureService* self, const char* id);

    /// Check whether a feature exists by its identifier.
    bool (*exists)(struct TifFeatureService* self, const char* id);

} TifFeatureService;

#endif // TIFERET_INTERFACES_FEATURE_H
