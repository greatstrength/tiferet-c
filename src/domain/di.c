// *** src/domain/di.c
// Implementation of TifFlaggedDependency and TifServiceConfiguration.

#include <tiferet/domain/di.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifFlaggedDependency

// ** function: tif_flagged_dep_init
TifResult tif_flagged_dep_init(TifFlaggedDependency* dep,
                               const char* module_path,
                               const char* class_name,
                               const char* flag) {

    // Zero-initialize.
    memset(dep, 0, sizeof(TifFlaggedDependency));

    // Copy strings.
    dep->module_path = tif_strdup(module_path);
    dep->class_name = tif_strdup(class_name);
    dep->flag = tif_strdup(flag);

    // Check allocations.
    if ((module_path && !dep->module_path) ||
        (class_name && !dep->class_name) ||
        (flag && !dep->flag)) {
        tif_flagged_dep_destroy(dep);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Initialize parameters map.
    tif_map_init(&dep->parameters);

    return TIF_OK;
}

// ** function: tif_flagged_dep_destroy
void tif_flagged_dep_destroy(TifFlaggedDependency* dep) {

    free(dep->module_path);
    free(dep->class_name);
    free(dep->flag);
    tif_map_destroy(&dep->parameters);

    memset(dep, 0, sizeof(TifFlaggedDependency));
}

// ** function: tif_flagged_dep_validate
bool tif_flagged_dep_validate(const TifFlaggedDependency* dep) {
    return !tif_str_is_empty(dep->module_path) &&
           !tif_str_is_empty(dep->class_name) &&
           !tif_str_is_empty(dep->flag);
}

// *** functions: TifServiceConfiguration

// ** function: tif_service_config_init
TifResult tif_service_config_init(TifServiceConfiguration* cfg,
                                  const char* id,
                                  const char* name,
                                  const char* module_path,
                                  const char* class_name) {

    // Zero-initialize.
    memset(cfg, 0, sizeof(TifServiceConfiguration));

    // Copy strings.
    cfg->id = tif_strdup(id);
    cfg->name = tif_strdup(name);
    cfg->module_path = tif_strdup(module_path);
    cfg->class_name = tif_strdup(class_name);

    // Check required allocation.
    if (id && !cfg->id) {
        tif_service_config_destroy(cfg);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Initialize parameters map and dependencies array.
    tif_map_init(&cfg->parameters);
    cfg->dependencies = NULL;
    cfg->dependency_count = 0;

    return TIF_OK;
}

// ** function: tif_service_config_destroy
void tif_service_config_destroy(TifServiceConfiguration* cfg) {

    free(cfg->id);
    free(cfg->name);
    free(cfg->module_path);
    free(cfg->class_name);
    tif_map_destroy(&cfg->parameters);

    // Free each flagged dependency.
    for (size_t i = 0; i < cfg->dependency_count; ++i) {
        tif_flagged_dep_destroy(&cfg->dependencies[i]);
    }
    free(cfg->dependencies);

    memset(cfg, 0, sizeof(TifServiceConfiguration));
}

// ** function: tif_service_config_validate
bool tif_service_config_validate(const TifServiceConfiguration* cfg) {
    return !tif_str_is_empty(cfg->id);
}

// ** function: tif_service_config_add_dependency
TifResult tif_service_config_add_dependency(TifServiceConfiguration* cfg,
                                            TifFlaggedDependency dep) {

    // Grow dependencies array.
    size_t new_count = cfg->dependency_count + 1;
    TifFlaggedDependency* new_deps = (TifFlaggedDependency*)realloc(
        cfg->dependencies, new_count * sizeof(TifFlaggedDependency));
    if (!new_deps) return TIF_ERR("ALLOC_FAILED");

    cfg->dependencies = new_deps;

    // Move the dependency into the array.
    cfg->dependencies[cfg->dependency_count] = dep;
    cfg->dependency_count = new_count;

    return TIF_OK;
}

// ** function: tif_service_config_get_dependency
const TifFlaggedDependency* tif_service_config_get_dependency(
    const TifServiceConfiguration* cfg,
    const char** flags,
    size_t flag_count) {

    // Check each flag in priority order.
    for (size_t f = 0; f < flag_count; ++f) {
        for (size_t d = 0; d < cfg->dependency_count; ++d) {
            if (cfg->dependencies[d].flag &&
                strcmp(cfg->dependencies[d].flag, flags[f]) == 0) {
                return &cfg->dependencies[d];
            }
        }
    }

    return NULL;
}
