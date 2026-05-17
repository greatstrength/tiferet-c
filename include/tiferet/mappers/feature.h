// *** tiferet/mappers/feature.h
// Feature mapper types for the Tiferet C framework.
// Mirrors C++ tiferet/mappers/feature.h.

#ifndef TIFERET_MAPPERS_FEATURE_H
#define TIFERET_MAPPERS_FEATURE_H

#include <tiferet/domain/feature.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** mappers

// ** mapper: tif_feature_event_aggregate
typedef struct { TifFeatureEvent data; } TifFeatureEventAggregate;

// ** mapper: tif_feature_aggregate
typedef struct { TifFeature data; } TifFeatureAggregate;

// ** mapper: tif_feature_event_yaml_object
typedef struct { TifFeatureEvent data; } TifFeatureEventYamlObject;

// ** mapper: tif_feature_yaml_object
typedef struct { TifFeature data; } TifFeatureYamlObject;

// *** functions: TifFeatureEventAggregate

TifResult tif_feat_event_agg_init(TifFeatureEventAggregate* agg,
                                  const char* name, const char* service_id,
                                  const char* data_key, bool pass_on_error,
                                  const char* condition);
void      tif_feat_event_agg_destroy(TifFeatureEventAggregate* agg);
TifResult tif_feat_event_agg_set_attribute(TifFeatureEventAggregate* agg,
                                           const char* attribute,
                                           const char* value);

// *** functions: TifFeatureAggregate

TifResult tif_feat_agg_init(TifFeatureAggregate* agg,
                            const char* id, const char* name,
                            const char* group_id, const char* feature_key,
                            const char* description);
void      tif_feat_agg_destroy(TifFeatureAggregate* agg);
TifResult tif_feat_agg_set_attribute(TifFeatureAggregate* agg,
                                     const char* attribute,
                                     const char* value);
TifResult tif_feat_agg_rename(TifFeatureAggregate* agg, const char* new_name);
TifResult tif_feat_agg_add_step(TifFeatureAggregate* agg,
                                TifFeatureEvent step);

// *** functions: TifFeatureEventYamlObject

TifResult tif_feat_event_yaml_to_map(const TifFeatureEventYamlObject* obj,
                                     TifMap* out, const char* role);
TifResult tif_feat_event_yaml_map(const TifFeatureEventYamlObject* obj,
                                  TifFeatureEventAggregate* out);
TifResult tif_feat_event_yaml_from_model(const TifFeatureEvent* model,
                                         TifFeatureEventYamlObject* out);

// *** functions: TifFeatureYamlObject

TifResult tif_feat_yaml_to_map(const TifFeatureYamlObject* obj,
                               TifMap* out, const char* role);
TifResult tif_feat_yaml_map(const TifFeatureYamlObject* obj,
                            TifFeatureAggregate* out);
TifResult tif_feat_yaml_from_model(const TifFeature* model,
                                   TifFeatureYamlObject* out);

#endif // TIFERET_MAPPERS_FEATURE_H
