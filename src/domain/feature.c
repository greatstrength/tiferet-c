// *** src/domain/feature.c
// Implementation of TifFeatureEvent and TifFeature domain functions.

#include <tiferet/domain/feature.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifFeatureEvent

// ** function: tif_feature_event_init
TifResult tif_feature_event_init(TifFeatureEvent* evt,
                                 const char* name,
                                 const char* service_id,
                                 const char* data_key,
                                 bool pass_on_error,
                                 const char* condition) {

    // Zero-initialize the struct.
    memset(evt, 0, sizeof(TifFeatureEvent));

    // Copy name.
    evt->name = tif_strdup(name);
    if (name && !evt->name) return TIF_ERR("ALLOC_FAILED");

    // Copy service_id.
    evt->service_id = tif_strdup(service_id);
    if (service_id && !evt->service_id) {
        tif_feature_event_destroy(evt);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Copy data_key (optional).
    if (data_key) {
        evt->data_key = tif_strdup(data_key);
        if (!evt->data_key) {
            tif_feature_event_destroy(evt);
            return TIF_ERR("ALLOC_FAILED");
        }
    }

    // Set pass_on_error flag.
    evt->pass_on_error = pass_on_error;

    // Copy condition (optional).
    if (condition) {
        evt->condition = tif_strdup(condition);
        if (!evt->condition) {
            tif_feature_event_destroy(evt);
            return TIF_ERR("ALLOC_FAILED");
        }
    }

    // Initialize the parameters map (empty).
    tif_map_init(&evt->parameters);

    return TIF_OK;
}

// ** function: tif_feature_event_destroy
void tif_feature_event_destroy(TifFeatureEvent* evt) {

    // Free owned strings.
    free(evt->name);
    free(evt->service_id);
    free(evt->data_key);
    free(evt->condition);

    // Destroy the parameters map.
    tif_map_destroy(&evt->parameters);

    // Reset all fields.
    memset(evt, 0, sizeof(TifFeatureEvent));
}

// ** function: tif_feature_event_validate
bool tif_feature_event_validate(const TifFeatureEvent* evt) {
    return !tif_str_is_empty(evt->name) &&
           !tif_str_is_empty(evt->service_id);
}

// *** functions: TifFeature

// ** function: tif_feature_init
TifResult tif_feature_init(TifFeature* feat,
                           const char* id,
                           const char* name,
                           const char* group_id,
                           const char* feature_key,
                           const char* description) {

    // Zero-initialize the struct.
    memset(feat, 0, sizeof(TifFeature));

    // Copy input strings into owned fields.
    feat->id = tif_strdup(id);
    feat->name = tif_strdup(name);
    feat->group_id = tif_strdup(group_id);
    feat->feature_key = tif_strdup(feature_key);
    feat->description = tif_strdup(description);

    // Derive any missing keys.
    tif_feature_derive_keys(&feat->id, &feat->group_id,
                            &feat->feature_key, name,
                            &feat->description);

    // Validate that derivation produced the required fields.
    if (tif_str_is_empty(feat->name)) {
        tif_feature_destroy(feat);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Steps start empty.
    feat->steps = NULL;
    feat->step_count = 0;

    return TIF_OK;
}

// ** function: tif_feature_destroy
void tif_feature_destroy(TifFeature* feat) {

    // Free owned strings.
    free(feat->id);
    free(feat->name);
    free(feat->description);
    free(feat->group_id);
    free(feat->feature_key);

    // Free each step in the array.
    for (size_t i = 0; i < feat->step_count; ++i) {
        tif_feature_event_destroy(&feat->steps[i]);
    }
    free(feat->steps);

    // Reset all fields.
    memset(feat, 0, sizeof(TifFeature));
}

// ** function: tif_feature_validate
bool tif_feature_validate(const TifFeature* feat) {
    return !tif_str_is_empty(feat->id) &&
           !tif_str_is_empty(feat->name) &&
           !tif_str_is_empty(feat->group_id) &&
           !tif_str_is_empty(feat->feature_key);
}

// ** function: tif_feature_add_step
TifResult tif_feature_add_step(TifFeature* feat, TifFeatureEvent step) {

    // Grow the steps array by one slot.
    size_t new_count = feat->step_count + 1;
    TifFeatureEvent* new_steps = (TifFeatureEvent*)realloc(
        feat->steps, new_count * sizeof(TifFeatureEvent));
    if (!new_steps) return TIF_ERR("ALLOC_FAILED");

    feat->steps = new_steps;

    // Move the step into the array (shallow copy — caller must not destroy).
    feat->steps[feat->step_count] = step;
    feat->step_count = new_count;

    return TIF_OK;
}

// ** function: tif_feature_get_step
const TifFeatureEvent* tif_feature_get_step(const TifFeature* feat,
                                            size_t position) {

    // Bounds check.
    if (position >= feat->step_count) return NULL;

    return &feat->steps[position];
}

// ** function: tif_feature_derive_keys
void tif_feature_derive_keys(char** id,
                             char** group_id,
                             char** feature_key,
                             const char* name,
                             char** description) {

    // Derive group_id and feature_key from dotted id.
    if (*id && !tif_str_is_empty(*id)) {
        const char* dot = strchr(*id, '.');
        if (dot) {
            if (tif_str_is_empty(*group_id)) {
                free(*group_id);
                size_t len = (size_t)(dot - *id);
                *group_id = (char*)malloc(len + 1);
                if (*group_id) {
                    memcpy(*group_id, *id, len);
                    (*group_id)[len] = '\0';
                }
            }
            if (tif_str_is_empty(*feature_key)) {
                free(*feature_key);
                *feature_key = tif_strdup(dot + 1);
            }
        }
    }

    // Derive feature_key from name (snake_case).
    if (!tif_str_is_empty(name) && tif_str_is_empty(*feature_key)) {
        free(*feature_key);
        *feature_key = tif_str_to_snake(name);
    }

    // Derive id from group_id and feature_key.
    if (tif_str_is_empty(*id) &&
        !tif_str_is_empty(*group_id) &&
        !tif_str_is_empty(*feature_key)) {
        free(*id);
        size_t g_len = strlen(*group_id);
        size_t k_len = strlen(*feature_key);
        *id = (char*)malloc(g_len + 1 + k_len + 1);
        if (*id) {
            memcpy(*id, *group_id, g_len);
            (*id)[g_len] = '.';
            memcpy(*id + g_len + 1, *feature_key, k_len);
            (*id)[g_len + 1 + k_len] = '\0';
        }
    }

    // Default description to name.
    if (!tif_str_is_empty(name) && tif_str_is_empty(*description)) {
        free(*description);
        *description = tif_strdup(name);
    }
}
