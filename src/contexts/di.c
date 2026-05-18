// *** src/contexts/di.c
// Implementation of TifDIContainer.

#include <tiferet/contexts/di.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>

// *** internal functions

// ** function: resolve_service (static)
/// Resolve a single TifServiceConfiguration into a live TifService.
/// Checks flagged dependencies first, then falls back to the default type.
static TifResult resolve_service(const TifServiceConfiguration* config,
                                 const char** flags,
                                 size_t flag_count,
                                 const TifServiceRegistry* registry,
                                 TifService** out) {

    const char* module_path = NULL;
    const char* class_name  = NULL;
    const TifMap* params    = NULL;

    // Check flagged dependencies first (in flag priority order).
    const TifFlaggedDependency* dep =
        tif_service_config_get_dependency(config, flags, flag_count);

    if (dep) {
        module_path = dep->module_path;
        class_name  = dep->class_name;
        params      = &dep->parameters;
    }

    // Fall back to the configuration's default module_path/class_name.
    if (!module_path || !class_name) {
        module_path = config->module_path;
        class_name  = config->class_name;
        params      = &config->parameters;
    }

    // Verify that both module_path and class_name are present.
    TIF_VERIFY(!tif_str_is_empty(module_path) && !tif_str_is_empty(class_name),
               TIF_ERR_SERVICE_RESOLUTION_FAILED,
               config->id);

    // Build the registry key ("module_path::class_name") and create the service.
    char* key = tif_str_make_key(module_path, class_name);
    TifResult result = tif_registry_create(registry, key, params, out);
    free(key);

    return result;
}

// *** functions

// ** function: tif_di_init
TifResult tif_di_init(TifDIContainer* di,
                      const TifServiceConfiguration* configs,
                      size_t config_count,
                      const char** flags,
                      size_t flag_count,
                      const TifServiceRegistry* registry) {

    // Initialize the empty services map.
    tif_map_init(&di->services);

    // Resolve each configuration into a live service instance.
    for (size_t i = 0; i < config_count; ++i) {
        TifService* svc = NULL;
        TifResult r = resolve_service(&configs[i], flags, flag_count,
                                      registry, &svc);

        // On failure, destroy already-resolved services and return the error.
        if (TIF_IS_ERR(r)) {
            tif_di_destroy(di);
            return r;
        }

        // Cache the resolved service by its configuration ID.
        tif_map_set(&di->services, configs[i].id, svc);
    }

    return TIF_OK;
}

// ** function: tif_di_destroy
void tif_di_destroy(TifDIContainer* di) {

    // Call destroy on each owned service before freeing the map.
    TifMapIter iter;
    tif_map_iter_init(&iter, &di->services);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        TifService* svc = (TifService*)value;
        if (svc && svc->destroy) {
            svc->destroy(svc);
        }
    }

    // Destroy the map, freeing all owned keys.
    tif_map_destroy(&di->services);
}

// ** function: tif_di_get
TifService* tif_di_get(const TifDIContainer* di, const char* id) {
    return (TifService*)tif_map_get(&di->services, id);
}

// ** function: tif_di_has
bool tif_di_has(const TifDIContainer* di, const char* id) {
    return tif_map_has(&di->services, id);
}

// ** function: tif_di_size
size_t tif_di_size(const TifDIContainer* di) {
    return tif_map_size(&di->services);
}
