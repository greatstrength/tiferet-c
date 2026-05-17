// *** tiferet/mappers/error.h
// Error mapper types for the Tiferet C framework.
// TifErrorAggregate provides mutable Error operations.
// TifErrorYamlObject provides serialization/mapping.
// Mirrors C++ tiferet/mappers/error.h.

#ifndef TIFERET_MAPPERS_ERROR_H
#define TIFERET_MAPPERS_ERROR_H

#include <tiferet/domain/error.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** mappers

// ** mapper: tif_error_aggregate
/// Mutable wrapper for TifError. Provides mutation functions.
typedef struct {
    TifError data;  // Embedded domain object.
} TifErrorAggregate;

// ** mapper: tif_error_yaml_object
/// Serialization wrapper for TifError. Provides to_map/map/from_model.
typedef struct {
    TifError data;  // Embedded domain object.
} TifErrorYamlObject;

// *** functions: TifErrorAggregate

/// Initialize an error aggregate. Delegates to tif_error_init.
TifResult tif_error_agg_init(TifErrorAggregate* agg,
                             const char* id,
                             const char* name,
                             const char* description,
                             const char* error_code);

/// Destroy an error aggregate.
void tif_error_agg_destroy(TifErrorAggregate* agg);

/// Set a named attribute. Dispatches to known Error fields.
/// Returns INVALID_MODEL_ATTRIBUTE for unknown attribute names.
TifResult tif_error_agg_set_attribute(TifErrorAggregate* agg,
                                      const char* attribute,
                                      const char* value);

/// Update the error name.
TifResult tif_error_agg_rename(TifErrorAggregate* agg, const char* new_name);

/// Set (or update) the message for a given language.
TifResult tif_error_agg_set_message(TifErrorAggregate* agg,
                                    const char* lang,
                                    const char* text);

/// Remove the message for a given language.
void tif_error_agg_remove_message(TifErrorAggregate* agg, const char* lang);

// *** functions: TifErrorYamlObject

/// Serialize scalar fields to a TifMap.
/// Roles: "" = all, "to_model" = all (messages handled by map),
///        "to_data" = exclude id.
TifResult tif_error_yaml_to_map(const TifErrorYamlObject* obj,
                                TifMap* out,
                                const char* role);

/// Map to an ErrorAggregate, carrying nested messages.
TifResult tif_error_yaml_map(const TifErrorYamlObject* obj,
                             TifErrorAggregate* out);

/// Create a YamlObject from an Error domain model.
TifResult tif_error_yaml_from_model(const TifError* model,
                                    TifErrorYamlObject* out);

#endif // TIFERET_MAPPERS_ERROR_H
