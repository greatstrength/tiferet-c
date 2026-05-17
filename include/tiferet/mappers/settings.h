// *** tiferet/mappers/settings.h
// Mapper conventions for the Tiferet C framework.
//
// The mappers layer bridges persistent configuration and runtime domain
// objects. It introduces two patterns, each implemented as a wrapper
// struct embedding the domain object via composition:
//
// 1. Aggregate — mutable wrapper for domain objects.
//    Provides mutation functions (set_attribute, rename, add_*, remove_*).
//    C++ uses multiple inheritance (Error + Aggregate); C uses composition:
//      typedef struct { TifError data; } TifErrorAggregate;
//
//    Convention:
//      TifResult tif_<type>_aggregate_init(...)     — init from field values
//      void      tif_<type>_aggregate_destroy(...)   — free owned fields
//      TifResult tif_<type>_aggregate_set_attribute(...) — name-based mutation
//      void/TifResult tif_<type>_aggregate_<verb>(...) — domain-specific mutation
//
// 2. TransferObject (YamlObject) — serialization wrapper for domain objects.
//    Provides serialization/mapping functions (to_map, map, from_model).
//    C++ uses multiple inheritance (Error + TransferObject); C uses composition:
//      typedef struct { TifError data; } TifErrorYamlObject;
//
//    Convention:
//      TifResult tif_<type>_yaml_to_map(...)   — serialize to TifMap with role
//      TifResult tif_<type>_yaml_map(...)       — map to aggregate
//      TifResult tif_<type>_yaml_from_model(...) — create from domain object
//
// Roles for to_map (mirrors Python _ROLES / C++ to_map role parameter):
//   ""         — all scalar fields (default)
//   "to_model" — fields needed to construct an aggregate (excludes nested)
//   "to_data"  — fields for persistent storage (may exclude derived keys)
//
// This header is documentation-only. It defines no types or functions.

#ifndef TIFERET_MAPPERS_SETTINGS_H
#define TIFERET_MAPPERS_SETTINGS_H

#endif // TIFERET_MAPPERS_SETTINGS_H
