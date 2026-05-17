// *** src/mappers/feature.c
// Implementation of Feature mapper functions.

#include <tiferet/mappers/feature.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifFeatureEventAggregate

// ** function: tif_feat_event_agg_init
TifResult tif_feat_event_agg_init(TifFeatureEventAggregate* agg,
                                  const char* name, const char* service_id,
                                  const char* data_key, bool pass_on_error,
                                  const char* condition) {
    return tif_feature_event_init(&agg->data, name, service_id,
                                  data_key, pass_on_error, condition);
}

// ** function: tif_feat_event_agg_destroy
void tif_feat_event_agg_destroy(TifFeatureEventAggregate* agg) {
    tif_feature_event_destroy(&agg->data);
}

// ** function: tif_feat_event_agg_set_attribute
TifResult tif_feat_event_agg_set_attribute(TifFeatureEventAggregate* agg,
                                           const char* attribute,
                                           const char* value) {

    if (strcmp(attribute, "name") == 0) {
        free(agg->data.name);
        agg->data.name = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "service_id") == 0) {
        free(agg->data.service_id);
        agg->data.service_id = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "data_key") == 0) {
        free(agg->data.data_key);
        agg->data.data_key = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "condition") == 0) {
        free(agg->data.condition);
        agg->data.condition = tif_strdup(value);
        return TIF_OK;
    }
    if (strcmp(attribute, "pass_on_error") == 0) {
        agg->data.pass_on_error = (value && strcmp(value, "false") != 0
                                        && strcmp(value, "0") != 0);
        return TIF_OK;
    }

    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}

// *** functions: TifFeatureAggregate

// ** function: tif_feat_agg_init
TifResult tif_feat_agg_init(TifFeatureAggregate* agg,
                            const char* id, const char* name,
                            const char* group_id, const char* feature_key,
                            const char* description) {
    return tif_feature_init(&agg->data, id, name, group_id,
                            feature_key, description);
}

// ** function: tif_feat_agg_destroy
void tif_feat_agg_destroy(TifFeatureAggregate* agg) {
    tif_feature_destroy(&agg->data);
}

// ** function: tif_feat_agg_set_attribute
TifResult tif_feat_agg_set_attribute(TifFeatureAggregate* agg,
                                     const char* attribute,
                                     const char* value) {

    if (strcmp(attribute, "name") == 0) {
        return tif_feat_agg_rename(agg, value);
    }
    if (strcmp(attribute, "description") == 0) {
        free(agg->data.description);
        agg->data.description = tif_strdup(value);
        return TIF_OK;
    }

    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}

// ** function: tif_feat_agg_rename
TifResult tif_feat_agg_rename(TifFeatureAggregate* agg, const char* new_name) {
    char* copy = tif_strdup(new_name);
    if (new_name && !copy) return TIF_ERR("ALLOC_FAILED");
    free(agg->data.name);
    agg->data.name = copy;
    return TIF_OK;
}

// ** function: tif_feat_agg_add_step
TifResult tif_feat_agg_add_step(TifFeatureAggregate* agg,
                                TifFeatureEvent step) {
    return tif_feature_add_step(&agg->data, step);
}

// *** functions: TifFeatureEventYamlObject

// ** function: tif_feat_event_yaml_to_map
TifResult tif_feat_event_yaml_to_map(const TifFeatureEventYamlObject* obj,
                                     TifMap* out, const char* role) {
    const TifFeatureEvent* e = &obj->data;
    (void)role;

    if (e->name) tif_map_set(out, "name", tif_strdup(e->name));
    if (e->service_id) tif_map_set(out, "service_id", tif_strdup(e->service_id));
    if (e->data_key) tif_map_set(out, "data_key", tif_strdup(e->data_key));
    tif_map_set(out, "pass_on_error", tif_strdup(e->pass_on_error ? "true" : "false"));
    if (e->condition) tif_map_set(out, "condition", tif_strdup(e->condition));

    return TIF_OK;
}

// ** function: tif_feat_event_yaml_map
TifResult tif_feat_event_yaml_map(const TifFeatureEventYamlObject* obj,
                                  TifFeatureEventAggregate* out) {
    const TifFeatureEvent* e = &obj->data;
    TIF_PROPAGATE(tif_feat_event_agg_init(out, e->name, e->service_id,
                                           e->data_key, e->pass_on_error,
                                           e->condition));

    // Copy parameters.
    TifMapIter iter;
    tif_map_iter_init(&iter, &e->parameters);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        tif_map_set(&out->data.parameters, key, tif_strdup((const char*)value));
    }

    return TIF_OK;
}

// ** function: tif_feat_event_yaml_from_model
TifResult tif_feat_event_yaml_from_model(const TifFeatureEvent* model,
                                         TifFeatureEventYamlObject* out) {
    TIF_PROPAGATE(tif_feature_event_init(&out->data, model->name,
                                          model->service_id, model->data_key,
                                          model->pass_on_error,
                                          model->condition));

    // Copy parameters.
    TifMapIter iter;
    tif_map_iter_init(&iter, &model->parameters);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        tif_map_set(&out->data.parameters, key, tif_strdup((const char*)value));
    }

    return TIF_OK;
}

// *** functions: TifFeatureYamlObject

// ** function: tif_feat_yaml_to_map
TifResult tif_feat_yaml_to_map(const TifFeatureYamlObject* obj,
                               TifMap* out, const char* role) {
    const TifFeature* f = &obj->data;

    if (role && strcmp(role, "to_data") == 0) {
        // Exclude derived keys for YAML storage.
        if (f->name) tif_map_set(out, "name", tif_strdup(f->name));
        if (f->description) tif_map_set(out, "description", tif_strdup(f->description));
    } else {
        if (f->id) tif_map_set(out, "id", tif_strdup(f->id));
        if (f->name) tif_map_set(out, "name", tif_strdup(f->name));
        if (f->group_id) tif_map_set(out, "group_id", tif_strdup(f->group_id));
        if (f->feature_key) tif_map_set(out, "feature_key", tif_strdup(f->feature_key));
        if (f->description) tif_map_set(out, "description", tif_strdup(f->description));
    }

    return TIF_OK;
}

// ** function: tif_feat_yaml_map
TifResult tif_feat_yaml_map(const TifFeatureYamlObject* obj,
                            TifFeatureAggregate* out) {
    const TifFeature* f = &obj->data;

    TIF_PROPAGATE(tif_feat_agg_init(out, f->id, f->name, f->group_id,
                                     f->feature_key, f->description));

    // Copy nested steps.
    for (size_t i = 0; i < f->step_count; ++i) {
        TifFeatureEvent step;
        TIF_PROPAGATE(tif_feature_event_init(&step, f->steps[i].name,
                                              f->steps[i].service_id,
                                              f->steps[i].data_key,
                                              f->steps[i].pass_on_error,
                                              f->steps[i].condition));

        // Copy step parameters.
        TifMapIter iter;
        tif_map_iter_init(&iter, &f->steps[i].parameters);
        const char* key;
        void* value;
        while (tif_map_iter_next(&iter, &key, &value)) {
            tif_map_set(&step.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_feat_agg_add_step(out, step));
    }

    return TIF_OK;
}

// ** function: tif_feat_yaml_from_model
TifResult tif_feat_yaml_from_model(const TifFeature* model,
                                   TifFeatureYamlObject* out) {
    TIF_PROPAGATE(tif_feature_init(&out->data, model->id, model->name,
                                    model->group_id, model->feature_key,
                                    model->description));

    // Copy steps.
    for (size_t i = 0; i < model->step_count; ++i) {
        TifFeatureEvent step;
        TIF_PROPAGATE(tif_feature_event_init(&step, model->steps[i].name,
                                              model->steps[i].service_id,
                                              model->steps[i].data_key,
                                              model->steps[i].pass_on_error,
                                              model->steps[i].condition));

        TifMapIter iter;
        tif_map_iter_init(&iter, &model->steps[i].parameters);
        const char* key;
        void* value;
        while (tif_map_iter_next(&iter, &key, &value)) {
            tif_map_set(&step.parameters, key, tif_strdup((const char*)value));
        }

        TIF_PROPAGATE(tif_feature_add_step(&out->data, step));
    }

    return TIF_OK;
}
