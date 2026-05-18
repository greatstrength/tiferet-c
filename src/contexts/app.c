// *** src/contexts/app.c
// Implementation of TifAppContext.

#include <tiferet/contexts/app.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>

// *** functions

// ** function: tif_app_ctx_init
void tif_app_ctx_init(TifAppContext* ctx,
                      TifFeatureService* feature_service,
                      TifErrorService* error_service,
                      const TifDIContainer* container) {

    // Store the borrowed feature service reference.
    ctx->feature_service = feature_service;

    // Initialize the embedded error context.
    tif_error_ctx_init(&ctx->error_ctx, error_service);

    // Initialize the embedded feature context.
    tif_feature_ctx_init(&ctx->feature_ctx, container);
}

// ** function: tif_app_ctx_run
TifResult tif_app_ctx_run(TifAppContext* ctx,
                          const char* feature_id,
                          TifMap* data) {

    // Look up the feature from the feature service.
    const TifFeature* feature =
        ctx->feature_service->get(ctx->feature_service, feature_id);

    // Raise an error if the feature is not found.
    TIF_VERIFY(feature != NULL,
               TIF_ERR_FEATURE_NOT_FOUND,
               feature_id);

    // Execute the feature pipeline.
    return tif_feature_ctx_execute(&ctx->feature_ctx, feature, data);
}

// ** function: tif_app_ctx_error_ctx
TifErrorContext* tif_app_ctx_error_ctx(TifAppContext* ctx) {
    return &ctx->error_ctx;
}
