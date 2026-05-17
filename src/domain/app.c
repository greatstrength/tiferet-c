// *** src/domain/app.c
// Implementation of TifAppServiceDependency and TifAppInterface.

#include <tiferet/domain/app.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifAppServiceDependency

// ** function: tif_app_service_dep_init
TifResult tif_app_service_dep_init(TifAppServiceDependency* dep,
                                   const char* service_id,
                                   const char* module_path,
                                   const char* class_name) {

    // Zero-initialize.
    memset(dep, 0, sizeof(TifAppServiceDependency));

    // Copy strings.
    dep->service_id = tif_strdup(service_id);
    dep->module_path = tif_strdup(module_path);
    dep->class_name = tif_strdup(class_name);

    // Check allocations.
    if ((service_id && !dep->service_id) ||
        (module_path && !dep->module_path) ||
        (class_name && !dep->class_name)) {
        tif_app_service_dep_destroy(dep);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Initialize parameters map.
    tif_map_init(&dep->parameters);

    return TIF_OK;
}

// ** function: tif_app_service_dep_destroy
void tif_app_service_dep_destroy(TifAppServiceDependency* dep) {

    free(dep->service_id);
    free(dep->module_path);
    free(dep->class_name);
    tif_map_destroy(&dep->parameters);

    memset(dep, 0, sizeof(TifAppServiceDependency));
}

// ** function: tif_app_service_dep_validate
bool tif_app_service_dep_validate(const TifAppServiceDependency* dep) {
    return !tif_str_is_empty(dep->service_id) &&
           !tif_str_is_empty(dep->module_path) &&
           !tif_str_is_empty(dep->class_name);
}

// *** functions: TifAppInterface

// ** function: tif_app_interface_init
TifResult tif_app_interface_init(TifAppInterface* iface,
                                 const char* id,
                                 const char* name,
                                 const char* module_path,
                                 const char* class_name,
                                 const char* description,
                                 const char* logger_id) {

    // Zero-initialize.
    memset(iface, 0, sizeof(TifAppInterface));

    // Copy required strings.
    iface->id = tif_strdup(id);
    iface->name = tif_strdup(name);

    // Copy optional strings.
    iface->module_path = tif_strdup(module_path);
    iface->class_name = tif_strdup(class_name);
    iface->description = tif_strdup(description);
    iface->logger_id = tif_strdup(logger_id ? logger_id : "default");

    // Check required allocations.
    if ((id && !iface->id) || (name && !iface->name) || !iface->logger_id) {
        tif_app_interface_destroy(iface);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Initialize constants map.
    tif_map_init(&iface->constants);

    // Default flag: "default".
    iface->flags = NULL;
    iface->flag_count = 0;
    TifResult r = tif_app_interface_add_flag(iface, "default");
    if (TIF_IS_ERR(r)) {
        tif_app_interface_destroy(iface);
        return r;
    }

    // Services start empty.
    iface->services = NULL;
    iface->service_count = 0;

    return TIF_OK;
}

// ** function: tif_app_interface_destroy
void tif_app_interface_destroy(TifAppInterface* iface) {

    free(iface->id);
    free(iface->name);
    free(iface->description);
    free(iface->module_path);
    free(iface->class_name);
    free(iface->logger_id);

    // Free flags array.
    for (size_t i = 0; i < iface->flag_count; ++i) {
        free(iface->flags[i]);
    }
    free(iface->flags);

    // Free services array.
    for (size_t i = 0; i < iface->service_count; ++i) {
        tif_app_service_dep_destroy(&iface->services[i]);
    }
    free(iface->services);

    // Destroy constants map.
    tif_map_destroy(&iface->constants);

    memset(iface, 0, sizeof(TifAppInterface));
}

// ** function: tif_app_interface_validate
bool tif_app_interface_validate(const TifAppInterface* iface) {
    return !tif_str_is_empty(iface->id) &&
           !tif_str_is_empty(iface->name);
}

// ** function: tif_app_interface_add_flag
TifResult tif_app_interface_add_flag(TifAppInterface* iface, const char* flag) {

    // Grow flags array.
    size_t new_count = iface->flag_count + 1;
    char** new_flags = (char**)realloc(iface->flags,
                                       new_count * sizeof(char*));
    if (!new_flags) return TIF_ERR("ALLOC_FAILED");

    iface->flags = new_flags;

    // Copy the flag string.
    iface->flags[iface->flag_count] = tif_strdup(flag);
    if (!iface->flags[iface->flag_count]) return TIF_ERR("ALLOC_FAILED");

    iface->flag_count = new_count;

    return TIF_OK;
}

// ** function: tif_app_interface_add_service
TifResult tif_app_interface_add_service(TifAppInterface* iface,
                                        TifAppServiceDependency dep) {

    // Grow services array.
    size_t new_count = iface->service_count + 1;
    TifAppServiceDependency* new_services = (TifAppServiceDependency*)realloc(
        iface->services, new_count * sizeof(TifAppServiceDependency));
    if (!new_services) return TIF_ERR("ALLOC_FAILED");

    iface->services = new_services;

    // Move the dependency into the array.
    iface->services[iface->service_count] = dep;
    iface->service_count = new_count;

    return TIF_OK;
}

// ** function: tif_app_interface_get_service
const TifAppServiceDependency* tif_app_interface_get_service(
    const TifAppInterface* iface, const char* service_id) {

    for (size_t i = 0; i < iface->service_count; ++i) {
        if (iface->services[i].service_id &&
            strcmp(iface->services[i].service_id, service_id) == 0) {
            return &iface->services[i];
        }
    }

    return NULL;
}
