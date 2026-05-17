// *** src/mappers/error.c
// Implementation of TifErrorAggregate and TifErrorYamlObject.

#include <tiferet/mappers/error.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifErrorAggregate

// ** function: tif_error_agg_init
TifResult tif_error_agg_init(TifErrorAggregate* agg,
                             const char* id,
                             const char* name,
                             const char* description,
                             const char* error_code) {

    return tif_error_init(&agg->data, id, name, description, error_code);
}

// ** function: tif_error_agg_destroy
void tif_error_agg_destroy(TifErrorAggregate* agg) {
    tif_error_destroy(&agg->data);
}

// ** function: tif_error_agg_set_attribute
TifResult tif_error_agg_set_attribute(TifErrorAggregate* agg,
                                      const char* attribute,
                                      const char* value) {

    // Dispatch to known fields.
    if (strcmp(attribute, "name") == 0) {
        return tif_error_agg_rename(agg, value);
    }

    if (strcmp(attribute, "description") == 0) {
        free(agg->data.description);
        agg->data.description = tif_strdup(value);
        return TIF_OK;
    }

    if (strcmp(attribute, "error_code") == 0) {
        free(agg->data.error_code);
        agg->data.error_code = tif_strdup(value);
        return TIF_OK;
    }

    // Unknown attribute.
    TIF_RAISE(TIF_ERR_INVALID_MODEL_ATTRIBUTE, attribute);
}

// ** function: tif_error_agg_rename
TifResult tif_error_agg_rename(TifErrorAggregate* agg, const char* new_name) {

    char* copy = tif_strdup(new_name);
    if (new_name && !copy) return TIF_ERR("ALLOC_FAILED");

    free(agg->data.name);
    agg->data.name = copy;

    return TIF_OK;
}

// ** function: tif_error_agg_set_message
TifResult tif_error_agg_set_message(TifErrorAggregate* agg,
                                    const char* lang,
                                    const char* text) {

    // Update existing message if a matching language is found.
    for (size_t i = 0; i < agg->data.message_count; ++i) {
        if (agg->data.messages[i].lang &&
            strcmp(agg->data.messages[i].lang, lang) == 0) {

            free(agg->data.messages[i].text);
            agg->data.messages[i].text = tif_strdup(text);
            return TIF_OK;
        }
    }

    // Not found — append a new message.
    return tif_error_add_message(&agg->data, lang, text);
}

// ** function: tif_error_agg_remove_message
void tif_error_agg_remove_message(TifErrorAggregate* agg, const char* lang) {

    for (size_t i = 0; i < agg->data.message_count; ++i) {
        if (agg->data.messages[i].lang &&
            strcmp(agg->data.messages[i].lang, lang) == 0) {

            // Destroy the message at index i.
            tif_error_message_destroy(&agg->data.messages[i]);

            // Shift subsequent messages left.
            size_t remaining = agg->data.message_count - i - 1;
            if (remaining > 0) {
                memmove(&agg->data.messages[i],
                        &agg->data.messages[i + 1],
                        remaining * sizeof(TifErrorMessage));
            }

            agg->data.message_count--;
            return;
        }
    }
}

// *** functions: TifErrorYamlObject

// ** function: tif_error_yaml_to_map
TifResult tif_error_yaml_to_map(const TifErrorYamlObject* obj,
                                TifMap* out,
                                const char* role) {

    const TifError* e = &obj->data;

    // Exclude id for "to_data" role.
    if ((!role || strcmp(role, "to_data") != 0) && e->id) {
        tif_map_set(out, "id", tif_strdup(e->id));
    }

    if (e->name) tif_map_set(out, "name", tif_strdup(e->name));
    if (e->description) tif_map_set(out, "description", tif_strdup(e->description));
    if (e->error_code) tif_map_set(out, "error_code", tif_strdup(e->error_code));

    return TIF_OK;
}

// ** function: tif_error_yaml_map
TifResult tif_error_yaml_map(const TifErrorYamlObject* obj,
                             TifErrorAggregate* out) {

    const TifError* e = &obj->data;

    // Initialize the aggregate from scalar fields.
    TIF_PROPAGATE(tif_error_agg_init(out, e->id, e->name,
                                     e->description, e->error_code));

    // Copy nested messages.
    for (size_t i = 0; i < e->message_count; ++i) {
        TIF_PROPAGATE(tif_error_add_message(&out->data,
                                            e->messages[i].lang,
                                            e->messages[i].text));
    }

    return TIF_OK;
}

// ** function: tif_error_yaml_from_model
TifResult tif_error_yaml_from_model(const TifError* model,
                                    TifErrorYamlObject* out) {

    // Initialize the yaml object's embedded domain object.
    TIF_PROPAGATE(tif_error_init(&out->data, model->id, model->name,
                                 model->description, model->error_code));

    // Copy messages.
    for (size_t i = 0; i < model->message_count; ++i) {
        TIF_PROPAGATE(tif_error_add_message(&out->data,
                                            model->messages[i].lang,
                                            model->messages[i].text));
    }

    return TIF_OK;
}
