// *** tiferet/domain/feature.h
// Feature domain models for the Tiferet C framework.
// TifFeatureEvent is a step in a feature workflow.
// TifFeature carries the full feature definition with key derivation.
// Mirrors C++ tiferet/domain/feature.h, C# FeatureConfiguration,
// and Python tiferet/domain/feature.py.

#ifndef TIFERET_DOMAIN_FEATURE_H
#define TIFERET_DOMAIN_FEATURE_H

#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

#include <stdbool.h>
#include <stddef.h>

// *** models

// ** model: tif_feature_event
/// A step in a feature workflow that executes a domain event.
typedef struct {
    char* name;             // Owned. Display name of the step.
    char* service_id;       // Owned. Service configuration ID.
    TifMap parameters;      // Owned. Step-level parameters (string→string).
    char* data_key;         // Owned. Optional key to store the result under.
    bool pass_on_error;     // Whether to continue execution on error.
    char* condition;        // Owned. Optional conditional expression.
} TifFeatureEvent;

// ** model: tif_feature
/// A feature definition with identifier, steps, and metadata.
typedef struct {
    char* id;               // Owned. Composite identifier (group_id.feature_key).
    char* name;             // Owned. Display name.
    char* description;      // Owned. Optional (defaults to name).
    char* group_id;         // Owned. Context group identifier.
    char* feature_key;      // Owned. Key within the group.
    TifFeatureEvent* steps; // Owned array of steps.
    size_t step_count;      // Number of steps.
} TifFeature;

// *** functions: TifFeatureEvent

/// Initialize a feature event. Copies all string inputs.
/// Parameters map is initialized empty.
TifResult tif_feature_event_init(TifFeatureEvent* evt,
                                 const char* name,
                                 const char* service_id,
                                 const char* data_key,
                                 bool pass_on_error,
                                 const char* condition);

/// Destroy a feature event, freeing all owned fields.
void tif_feature_event_destroy(TifFeatureEvent* evt);

/// Validate a feature event. name and service_id must be non-empty.
bool tif_feature_event_validate(const TifFeatureEvent* evt);

// *** functions: TifFeature

/// Initialize a feature. Copies all string inputs.
/// If id is NULL, it is derived from group_id and feature_key.
/// If feature_key is NULL, it is derived from name (snake_case).
/// If description is NULL, it defaults to name.
/// Steps are not set here — use tif_feature_add_step after init.
TifResult tif_feature_init(TifFeature* feat,
                           const char* id,
                           const char* name,
                           const char* group_id,
                           const char* feature_key,
                           const char* description);

/// Destroy a feature, freeing all owned fields and steps.
void tif_feature_destroy(TifFeature* feat);

/// Validate a feature. id, name, group_id, and feature_key must be non-empty.
bool tif_feature_validate(const TifFeature* feat);

/// Add a step to the feature. The event is moved (shallow copy),
/// so the caller should NOT destroy the source event after this call.
TifResult tif_feature_add_step(TifFeature* feat, TifFeatureEvent step);

/// Get the step at the given position, or NULL if out of range.
const TifFeatureEvent* tif_feature_get_step(const TifFeature* feat,
                                            size_t position);

/// Derive id, group_id, feature_key, and description from
/// whichever inputs are provided. Mirrors Python Feature.derive_keys.
/// All parameters are in/out — non-NULL owned strings.
/// Caller is responsible for freeing any newly allocated strings.
void tif_feature_derive_keys(char** id,
                             char** group_id,
                             char** feature_key,
                             const char* name,
                             char** description);

#endif // TIFERET_DOMAIN_FEATURE_H
