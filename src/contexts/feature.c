// *** src/contexts/feature.c
// Implementation of TifFeatureContext.

#include <tiferet/contexts/feature.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>

#include <stdlib.h>

// *** internal functions

// ** function: run_step (static)
/// Execute a single feature step and store the result if data_key is set.
static TifResult run_step(TifExecutableEvent* event,
                          const TifFeatureEvent* step,
                          TifMap* data) {

    // Execute the event with the data context and step parameters.
    char* result = NULL;
    TifResult r = event->execute(event, data, &step->parameters, &result);

    // Return immediately on error (free any partial result).
    if (TIF_IS_ERR(r)) {
        free(result);
        return r;
    }

    // Store the result under data_key if both are present and non-empty.
    if (step->data_key && step->data_key[0] != '\0' && result) {
        tif_map_set(data, step->data_key, result);
    } else {
        // No data_key or no result — discard the allocated string.
        free(result);
    }

    return TIF_OK;
}

// *** functions

// ** function: tif_feature_ctx_init
void tif_feature_ctx_init(TifFeatureContext* ctx,
                          const TifDIContainer* container) {

    // Store the borrowed container reference.
    ctx->container = container;
}

// ** function: tif_feature_ctx_execute
TifResult tif_feature_ctx_execute(TifFeatureContext* ctx,
                                  const TifFeature* feature,
                                  TifMap* data) {

    // Iterate over the feature's configured steps.
    for (size_t i = 0; i < feature->step_count; ++i) {
        const TifFeatureEvent* step = &feature->steps[i];

        // Resolve the service from the DI container by service_id.
        TifService* svc = tif_di_get(ctx->container, step->service_id);

        // Raise an error if the service is not found in the container.
        TIF_VERIFY(svc != NULL,
                   TIF_ERR_FEATURE_COMMAND_LOADING_FAILED,
                   step->service_id);

        // Cast the service to TifExecutableEvent for uniform invocation.
        TifExecutableEvent* event = (TifExecutableEvent*)svc;

        // Verify the event implements the execute function pointer.
        TIF_VERIFY(event->execute != NULL,
                   TIF_ERR_FEATURE_STEP_EXECUTION_FAILED,
                   step->service_id);

        // Execute the step and handle errors according to pass_on_error.
        TifResult r = run_step(event, step, data);

        // Propagate failure unless pass_on_error is set.
        if (TIF_IS_ERR(r) && !step->pass_on_error) {
            return r;
        }
    }

    return TIF_OK;
}
