// *** tiferet/mappers/app.h
// App mapper types for the Tiferet C framework.
// Mirrors C++ tiferet/mappers/app.h.

#ifndef TIFERET_MAPPERS_APP_H
#define TIFERET_MAPPERS_APP_H

#include <tiferet/domain/app.h>
#include <tiferet/assets/errors.h>
#include <tiferet/utils/map.h>

// *** mappers

// ** mapper: tif_app_interface_aggregate
typedef struct { TifAppInterface data; } TifAppInterfaceAggregate;

// ** mapper: tif_app_interface_yaml_object
typedef struct { TifAppInterface data; } TifAppInterfaceYamlObject;

// *** functions: TifAppInterfaceAggregate

TifResult tif_app_iface_agg_init(TifAppInterfaceAggregate* agg,
                                 const char* id, const char* name,
                                 const char* module_path,
                                 const char* class_name,
                                 const char* description,
                                 const char* logger_id);
void      tif_app_iface_agg_destroy(TifAppInterfaceAggregate* agg);
TifResult tif_app_iface_agg_set_attribute(TifAppInterfaceAggregate* agg,
                                          const char* attribute,
                                          const char* value);
TifResult tif_app_iface_agg_add_service(TifAppInterfaceAggregate* agg,
                                        TifAppServiceDependency dep);

// *** functions: TifAppInterfaceYamlObject

TifResult tif_app_iface_yaml_to_map(const TifAppInterfaceYamlObject* obj,
                                    TifMap* out, const char* role);
TifResult tif_app_iface_yaml_map(const TifAppInterfaceYamlObject* obj,
                                 TifAppInterfaceAggregate* out);
TifResult tif_app_iface_yaml_from_model(const TifAppInterface* model,
                                        TifAppInterfaceYamlObject* out);

#endif // TIFERET_MAPPERS_APP_H
