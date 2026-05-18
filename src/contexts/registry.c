// *** src/contexts/registry.c
// Implementation of TifServiceRegistry.
// Uses a TifMap with heap-allocated TifFactoryWrapper values to store
// factory function pointers without void* <-> function pointer casts.

#include <tiferet/contexts/registry.h>
#include <tiferet/assets/constants.h>
#include <tiferet/events/settings.h>

#include <stdlib.h>

// *** internal types

// ** type: tif_factory_wrapper
/// Heap-allocated wrapper for TifFactoryFn.
/// Avoids ISO C-prohibited casts between function pointers and void*.
typedef struct {
    TifFactoryFn factory;
} TifFactoryWrapper;

// *** functions

// ** function: tif_registry_init
void tif_registry_init(TifServiceRegistry* reg) {

    // Initialize the empty factories map.
    tif_map_init(&reg->factories);
}

// ** function: tif_registry_destroy
void tif_registry_destroy(TifServiceRegistry* reg) {

    // Free each factory wrapper value before destroying the map.
    TifMapIter iter;
    tif_map_iter_init(&iter, &reg->factories);
    const char* key;
    void* value;
    while (tif_map_iter_next(&iter, &key, &value)) {
        free(value);
    }

    // Destroy the map, freeing all owned keys.
    tif_map_destroy(&reg->factories);
}

// ** function: tif_registry_register
bool tif_registry_register(TifServiceRegistry* reg,
                           const char* key,
                           TifFactoryFn factory) {

    // If a wrapper already exists for this key, update it in place.
    TifFactoryWrapper* existing =
        (TifFactoryWrapper*)tif_map_get(&reg->factories, key);
    if (existing) {
        existing->factory = factory;
        return true;
    }

    // Allocate a new wrapper for the factory function.
    TifFactoryWrapper* wrapper =
        (TifFactoryWrapper*)malloc(sizeof(TifFactoryWrapper));
    if (!wrapper) return false;
    wrapper->factory = factory;

    // Store the wrapper in the factories map.
    if (!tif_map_set(&reg->factories, key, wrapper)) {
        free(wrapper);
        return false;
    }

    return true;
}

// ** function: tif_registry_create
TifResult tif_registry_create(const TifServiceRegistry* reg,
                              const char* key,
                              const TifMap* params,
                              TifService** out) {

    // Look up the factory wrapper for the given key.
    TifFactoryWrapper* wrapper =
        (TifFactoryWrapper*)tif_map_get(&reg->factories, key);

    // Return an error if the key is not registered.
    TIF_VERIFY(wrapper != NULL,
               TIF_ERR_SERVICE_NOT_REGISTERED,
               key);

    // Invoke the factory with the parameter map.
    TifService* svc = wrapper->factory(params);

    // Return an error if the factory returned NULL.
    TIF_VERIFY(svc != NULL,
               TIF_ERR_SERVICE_RESOLUTION_FAILED,
               key);

    // Set the output service pointer.
    *out = svc;
    return TIF_OK;
}

// ** function: tif_registry_has
bool tif_registry_has(const TifServiceRegistry* reg, const char* key) {
    return tif_map_has(&reg->factories, key);
}

// ** function: tif_registry_size
size_t tif_registry_size(const TifServiceRegistry* reg) {
    return tif_map_size(&reg->factories);
}
