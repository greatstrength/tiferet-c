// *** src/mappers/app.c
// Implementation of App mapper functions.

#include <tiferet/mappers/app.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifAppInterfaceAggregate

// ** function: tif_app_iface_agg_init
TifResult tif_app_iface_agg_init(TifAppInterfaceAggregate* agg,
                                 const char* id, const char* name,
                                 const char* module_path,
                                 const char* class_name,
                                 const char* description,
                                 const char* logger_id) {
    return tif_app_interface_init(&agg->data, id, name, module_path,
                                  class_name, description, logger_id);
}

// ** function: tif_app_iface_agg_destroy
void tif_app_iface_agg_destroy(TifAppInterfaceAggregate* agg) {
    tif_app_interface_destroy(&agg->data);
}

// ** function: tif_app_iface_agg_set_attribute
TifResult tif_app_iface_agg_set_attribute(TifAppInterfaceAggregate* agg,
                                          const char* attribute,
                                          const char* value) {
    if (strcmp(attribute, "name") == 0) {
        free(agg->data.name);
        agg->data.name = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "description") == 0) {
        free(agg->data.description);
        agg->data.description = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "module_path") == 0) {
        free(agg->data.module_path);
        agg->data.module_path = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "class_name") == 0) {
        free(agg->data.class_name);
        agg->data.class_name = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "logger_id") == 0) {
        free(agg->data.logger_id);
        agg->data.logger_id = tif_strdup(value);
        return TIF_OK;
    }

    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}

// ** function: tif_app_iface_agg_add_service
TifResult tif_app_iface_agg_add_service(TifAppInterfaceAggregate* agg,
                                        TifAppServiceDependency dep) {
    return tif_app_interface_add_service(&agg->data, dep);
}

// *** functions: TifAppInterfaceYamlObject

// ** function: tif_app_iface_yaml_to_map
TifResult tif_app_iface_yaml_to_map(const TifAppInterfaceYamlObject* obj,
                                    TifMap* out, const char* role) {
    const TifAppInterface* a = &obj->data;
    (void)role;

    if (a->id) tif_map_set(out, "id", tif_strdup(a->id));
    if (a->name) tif_map_set(out, "name", tif_strdup(a->name));
    if (a->description) tif_map_set(out, "description", tif_strdup(a->description));
    if (a->module_path) tif_map_set(out, "module_path", tif_strdup(a->module_path));
    if (a->class_name) tif_map_set(out, "class_name", tif_strdup(a->class_name));
    if (a->logger_id) tif_map_set(out, "logger_id", tif_strdup(a->logger_id));

    return TIF_OK;
}

// ** function: tif_app_iface_yaml_map
TifResult tif_app_iface_yaml_map(const TifAppInterfaceYamlObject* obj,
                                 TifAppInterfaceAggregate* out) {
    const TifAppInterface* a = &obj->data;

    TIF_PROPAGATE(tif_app_iface_agg_init(out, a->id, a->name,
                                          a->module_path, a->class_name,
                                          a->description, a->logger_id));

    // Copy services.
    for (size_t i = 0; i < a->service_count; ++i) {
        TifAppServiceDependency dep;
        TIF_PROPAGATE(tif_app_service_dep_init(&dep,
                                                a->services[i].service_id,
                                                a->services[i].module_path,
                                                a->services[i].class_name));

        // Copy service parameters.
        TifMapIter iter;
        tif_map_iter_init(&iter, &a->services[i].parameters);
        const char* key;
        void* value;
        while (tif_map_iter_next(&iter, &key, &value)) {
            tif_map_set(&dep.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_app_iface_agg_add_service(out, dep));
    }

    return TIF_OK;
}

// ** function: tif_app_iface_yaml_from_model
TifResult tif_app_iface_yaml_from_model(const TifAppInterface* model,
                                        TifAppInterfaceYamlObject* out) {
    TIF_PROPAGATE(tif_app_interface_init(&out->data, model->id, model->name,
                                          model->module_path, model->class_name,
                                          model->description, model->logger_id));

    // Copy services.
    for (size_t i = 0; i < model->service_count; ++i) {
        TifAppServiceDependency dep;
        TIF_PROPAGATE(tif_app_service_dep_init(&dep,
                                                model->services[i].service_id,
                                                model->services[i].module_path,
                                                model->services[i].class_name));

        TifMapIter iter;
        tif_map_iter_init(&iter, &model->services[i].parameters);
        const char* key;
        void* value;
        while (tif_map_iter_next(&iter, &key, &value)) {
            tif_map_set(&dep.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_app_interface_add_service(&out->data, dep));
    }

    return TIF_OK;
}
