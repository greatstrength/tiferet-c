// *** tiferet/mappers/di.h
// DI mapper types for the Tiferet C framework.
// Mirrors C++ tiferet/mappers/di.h.

#ifndef TIFERET_MAPPERS_DI_H
#define TIFERET_MAPPERS_DI_H

#include <tiferet/domain/di.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** mappers

// ** mapper: tif_service_config_aggregate
typedef struct { TifServiceConfiguration data; } TifServiceConfigAggregate;

// ** mapper: tif_service_config_yaml_object
typedef struct { TifServiceConfiguration data; } TifServiceConfigYamlObject;

// *** functions: TifServiceConfigAggregate

TifResult tif_svc_cfg_agg_init(TifServiceConfigAggregate* agg,
                               const char* id, const char* name,
                               const char* module_path,
                               const char* class_name);
void      tif_svc_cfg_agg_destroy(TifServiceConfigAggregate* agg);
TifResult tif_svc_cfg_agg_set_attribute(TifServiceConfigAggregate* agg,
                                        const char* attribute,
                                        const char* value);
TifResult tif_svc_cfg_agg_add_dependency(TifServiceConfigAggregate* agg,
                                         TifFlaggedDependency dep);

// *** functions: TifServiceConfigYamlObject

TifResult tif_svc_cfg_yaml_to_map(const TifServiceConfigYamlObject* obj,
                                  TifMap* out, const char* role);
TifResult tif_svc_cfg_yaml_map(const TifServiceConfigYamlObject* obj,
                               TifServiceConfigAggregate* out);
TifResult tif_svc_cfg_yaml_from_model(const TifServiceConfiguration* model,
                                      TifServiceConfigYamlObject* out);

#endif // TIFERET_MAPPERS_DI_H
