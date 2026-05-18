// *** tiferet/contexts/feature.h
// Feature execution pipeline for the Tiferet C framework.
// TifExecutableEvent is a vtable struct for events in feature pipelines.
// TifFeatureContext iterates a feature's steps and executes them.
// Mirrors C++ ExecutableEvent and FeatureContext.

#ifndef TIFERET_CONTEXTS_FEATURE_H
#define TIFERET_CONTEXTS_FEATURE_H

#include <tiferet/contexts/di.h>
#include <tiferet/domain/feature.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** interfaces

// ** interface: tif_executable_event
/// Vtable interface for domain events that participate in feature pipelines.
/// Extends TifService with a string-map execute() entry point so that
/// TifFeatureContext can invoke them uniformly.
///
/// Concrete events embed this struct, set base.destroy to their cleanup
/// function, and set execute to their implementation.
///
/// IMPORTANT: All services registered for feature steps must implement this
/// interface. The execute function pointer must not be NULL.
///
/// Mirrors C++ ExecutableEvent.
typedef struct TifExecutableEvent {

    /// Base service vtable. Must be the first member.
    TifService base;

    /// Execute the event with the current data context and step parameters.
    ///
    /// If a result is produced, the implementation allocates a heap string
    /// and sets *result_out to it. The caller (TifFeatureContext) takes
    /// ownership and stores it in the data map or frees it.
    /// If no result is produced, *result_out must be set to NULL.
    ///
    /// :param self:       The event instance.
    /// :param data:       The current data context (read-only, string->string).
    /// :param params:     Step-level parameters from TifFeatureEvent.parameters.
    /// :param result_out: Output for a heap-allocated result string, or NULL.
    /// :return:           TIF_OK on success, or an error result.
    TifResult (*execute)(struct TifExecutableEvent* self,
                         const TifMap* data,
                         const TifMap* params,
                         char** result_out);

} TifExecutableEvent;

// *** contexts

// ** context: tif_feature_context
/// Orchestrates execution of a TifFeature's step pipeline.
/// For each step: resolves the service from the DI container by service_id,
/// casts to TifExecutableEvent*, merges step parameters with data, calls
/// execute(), and stores the result under data_key if specified.
/// Mirrors C++ FeatureContext.
typedef struct {
    const TifDIContainer* container;    // Borrowed. DI container for step resolution.
} TifFeatureContext;

// *** functions

/// Initialize a feature context with a reference to the DI container.
///
/// :param ctx:       The feature context to initialize.
/// :param container: Borrowed reference to the DI container.
void tif_feature_ctx_init(TifFeatureContext* ctx,
                          const TifDIContainer* container);

/// Execute a feature by iterating its steps.
///
/// For each step:
///   1. Resolves the service from the DI container by service_id.
///   2. Casts to TifExecutableEvent*.
///   3. Calls execute() with the data context and step parameters.
///   4. Stores the result string in data under data_key (if both are present).
///   5. Honors pass_on_error — continues on failure when true.
///
/// Result strings stored in data are heap-allocated; the caller owns them
/// and must free them after use.
///
/// :param ctx:     The feature context.
/// :param feature: The feature definition to execute.
/// :param data:    The data context map (string->string*), modified in place.
/// :return:        TIF_OK on success, or the first non-pass_on_error error.
TifResult tif_feature_ctx_execute(TifFeatureContext* ctx,
                                  const TifFeature* feature,
                                  TifMap* data);

#endif // TIFERET_CONTEXTS_FEATURE_H
