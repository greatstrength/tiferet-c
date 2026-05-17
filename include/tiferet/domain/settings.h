// *** tiferet/domain/settings.h
// Domain object conventions for the Tiferet C framework.
//
// Domain objects are plain C structs with associated free functions.
// They are read-only value types — mutation logic lives on Aggregate
// wrappers in the mappers layer.
//
// Every domain object follows this lifecycle pattern:
//
//   TifResult tif_<type>_init(Tif<Type>* obj, ...);
//     - Allocates and copies all owned fields (strings, arrays).
//     - Returns TIF_OK on success, TIF_ERR on allocation failure.
//
//   void tif_<type>_destroy(Tif<Type>* obj);
//     - Frees all owned fields and resets them to NULL/0.
//     - Safe to call on a zero-initialized struct.
//
//   bool tif_<type>_validate(const Tif<Type>* obj);
//     - Returns true if all required fields are non-empty.
//     - Does not modify the object.
//
// Ownership rule:
//   All char* fields in domain structs are OWNED — allocated via
//   tif_strdup() during init, freed during destroy. Functions that
//   accept const char* inputs always copy them.
//
// This header is documentation-only. It defines no types or functions.

#ifndef TIFERET_DOMAIN_SETTINGS_H
#define TIFERET_DOMAIN_SETTINGS_H

#endif // TIFERET_DOMAIN_SETTINGS_H
