// *** tiferet/tiferet.h
// Umbrella header for the Tiferet C framework.
// Include this single header to access the full public API surface.

#ifndef TIFERET_H
#define TIFERET_H

// *** assets — error codes, result types
#include <tiferet/assets/constants.h>
#include <tiferet/assets/errors.h>

// *** domain — domain object base and domain models
#include <tiferet/domain/settings.h>
#include <tiferet/domain/error.h>
#include <tiferet/domain/feature.h>
#include <tiferet/domain/app.h>
#include <tiferet/domain/di.h>

// *** events — domain event infrastructure (verify/raise macros)
#include <tiferet/events/settings.h>

// *** interfaces — service contracts (vtable structs)
#include <tiferet/interfaces/settings.h>
#include <tiferet/interfaces/feature.h>
#include <tiferet/interfaces/error.h>
#include <tiferet/interfaces/container.h>
#include <tiferet/interfaces/app.h>

// *** mappers — aggregates and transfer objects
#include <tiferet/mappers/settings.h>
#include <tiferet/mappers/error.h>
#include <tiferet/mappers/feature.h>
#include <tiferet/mappers/app.h>
#include <tiferet/mappers/di.h>

// *** contexts — runtime orchestration
#include <tiferet/contexts/registry.h>
#include <tiferet/contexts/di.h>
#include <tiferet/contexts/feature.h>
#include <tiferet/contexts/error.h>
#include <tiferet/contexts/app.h>

// *** utils — string, vector, and map utilities
#include <tiferet/utils/strutil.h>
#include <tiferet/utils/vector.h>
#include <tiferet/utils/map.h>

#endif // TIFERET_H
