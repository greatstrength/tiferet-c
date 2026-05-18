// *** tiferet/contexts/app.h
// App interface context for the Tiferet C framework.
// Ties together TifFeatureService, TifErrorContext, and TifFeatureContext
// to provide a top-level tif_app_ctx_run() entry point.
// Mirrors C++ AppInterfaceContext and Python AppInterfaceContext.

#ifndef TIFERET_CONTEXTS_APP_H
#define TIFERET_CONTEXTS_APP_H

#include <tiferet/contexts/feature.h>
#include <tiferet/contexts/error.h>
#include <tiferet/interfaces/feature.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** contexts

// ** context: tif_app_context
/// Top-level orchestration context for executing features.
/// Combines TifFeatureService (feature lookup), TifFeatureContext (pipeline
/// execution), and TifErrorContext (error formatting) into a single
/// tif_app_ctx_run() entry point.
/// Mirrors C++ AppInterfaceContext.
///
/// Usage:
///   TifAppContext app;
///   tif_app_ctx_init(&app, feature_svc, error_svc, &di_container);
///   TifMap data;
///   tif_map_init(&data);
///   tif_map_set(&data, "a", "3");
///   tif_map_set(&data, "b", "4");
///   TifResult r = tif_app_ctx_run(&app, "calc.add", &data);
///   // data["result"] == "7"  (heap-allocated, caller frees)
///   tif_map_destroy(&data);
typedef struct {
    TifFeatureService* feature_service; // Borrowed. Feature service for lookups.
    TifErrorContext    error_ctx;       // Embedded. Error context for formatting.
    TifFeatureContext  feature_ctx;     // Embedded. Feature execution pipeline.
} TifAppContext;

// *** functions

/// Initialize an app context with all required services.
///
/// :param ctx:             The app context to initialize.
/// :param feature_service: Borrowed reference to the feature service.
/// :param error_service:   Borrowed reference to the error service.
/// :param container:       Borrowed reference to the DI container.
void tif_app_ctx_init(TifAppContext* ctx,
                      TifFeatureService* feature_service,
                      TifErrorService* error_service,
                      const TifDIContainer* container);

/// Execute a feature by its composite identifier.
/// Looks up the feature from the TifFeatureService and executes its
/// step pipeline via TifFeatureContext. Results are stored in the data map.
///
/// Returns TIF_ERR_FEATURE_NOT_FOUND if the feature does not exist.
///
/// :param ctx:        The app context.
/// :param feature_id: The composite feature identifier (e.g., "calc.add").
/// :param data:       The data context map, modified in place as steps run.
/// :return:           TIF_OK on success, or an error result.
TifResult tif_app_ctx_run(TifAppContext* ctx,
                          const char* feature_id,
                          TifMap* data);

/// Access the embedded error context for error formatting.
///
/// :param ctx: The app context.
/// :return:    Pointer to the embedded TifErrorContext.
TifErrorContext* tif_app_ctx_error_ctx(TifAppContext* ctx);

#endif // TIFERET_CONTEXTS_APP_H
