// *** src/mappers/di.c
// Implementation of DI mapper functions.

#include <tiferet/mappers/di.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifServiceConfigAggregate

// ** function: tif_svc_cfg_agg_init
TifResult tif_svc_cfg_agg_init(TifServiceConfigAggregate* agg,
                               const char* id, const char* name,
                               const char* module_path,
                               const char* class_name) {
    return tif_service_config_init(&agg->data, id, name,
                                   module_path, class_name);
}

// ** function: tif_svc_cfg_agg_destroy
void tif_svc_cfg_agg_destroy(TifServiceConfigAggregate* agg) {
    tif_service_config_destroy(&agg->data);
}

// ** function: tif_svc_cfg_agg_set_attribute
TifResult tif_svc_cfg_agg_set_attribute(TifServiceConfigAggregate* agg,
                                        const char* attribute,
                                        const char* value) {
    if (strcmp(attribute, "name") == 0) {
        free(agg->data.name);
        agg->data.name = tif_strdup(value);
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

    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}

// ** function: tif_svc_cfg_agg_add_dependency
TifResult tif_svc_cfg_agg_add_dependency(TifServiceConfigAggregate* agg,
                                         TifFlaggedDependency dep) {
    return tif_service_config_add_dependency(&agg->data, dep);
}

// *** functions: TifServiceConfigYamlObject

// ** function: tif_svc_cfg_yaml_to_map
TifResult tif_svc_cfg_yaml_to_map(const TifServiceConfigYamlObject* obj,
                                  TifMap* out, const char* role) {
    const TifServiceConfiguration* c = &obj->data;
    (void)role;

    if (c->id) tif_map_set(out, "id", tif_strdup(c->id));
    if (c->name) tif_map_set(out, "name", tif_strdup(c->name));
    if (c->module_path) tif_map_set(out, "module_path", tif_strdup(c->module_path));
    if (c->class_name) tif_map_set(out, "class_name", tif_strdup(c->class_name));

    return TIF_OK;
}

// ** function: tif_svc_cfg_yaml_map
TifResult tif_svc_cfg_yaml_map(const TifServiceConfigYamlObject* obj,
                               TifServiceConfigAggregate* out) {
    const TifServiceConfiguration* c = &obj->data;

    TIF_PROPAGATE(tif_svc_cfg_agg_init(out, c->id, c->name,
                                        c->module_path, c->class_name));

    // Copy parameters.
    TifMapIter iter;
    tif_map_iter_init(&iter, &c->parameters);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        tif_map_set(&out->data.parameters, key, tif_strdup((const char*)value));
    }

    // Copy flagged dependencies.
    for (size_t i = 0; i < c->dependency_count; ++i) {
        TifFlaggedDependency dep;
        TIF_PROPAGATE(tif_flagged_dep_init(&dep,
                                            c->dependencies[i].module_path,
                                            c->dependencies[i].class_name,
                                            c->dependencies[i].flag));

        // Copy dependency parameters.
        TifMapIter dep_iter;
        tif_map_iter_init(&dep_iter, &c->dependencies[i].parameters);
        while (tif_map_iter_next(&dep_iter, &key, &value)) {
            tif_map_set(&dep.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_svc_cfg_agg_add_dependency(out, dep));
    }

    return TIF_OK;
}

// ** function: tif_svc_cfg_yaml_from_model
TifResult tif_svc_cfg_yaml_from_model(const TifServiceConfiguration* model,
                                      TifServiceConfigYamlObject* out) {

    TIF_PROPAGATE(tif_service_config_init(&out->data, model->id, model->name,
                                           model->module_path,
                                           model->class_name));

    // Copy parameters.
    TifMapIter iter;
    tif_map_iter_init(&iter, &model->parameters);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        tif_map_set(&out->data.parameters, key, tif_strdup((const char*)value));
    }

    // Copy flagged dependencies.
    for (size_t i = 0; i < model->dependency_count; ++i) {
        TifFlaggedDependency dep;
        TIF_PROPAGATE(tif_flagged_dep_init(&dep,
                                            model->dependencies[i].module_path,
                                            model->dependencies[i].class_name,
                                            model->dependencies[i].flag));

        TifMapIter dep_iter;
        tif_map_iter_init(&dep_iter, &model->dependencies[i].parameters);
        while (tif_map_iter_next(&dep_iter, &key, &value)) {
            tif_map_set(&dep.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_service_config_add_dependency(&out->data, dep));
    }

    return TIF_OK;
}
