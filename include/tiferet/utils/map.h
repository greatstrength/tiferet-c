// *** tiferet/utils/map.h
// String-keyed hash map for the Tiferet C framework.
// Uses open addressing with linear probing.
// Keys are owned (copied on set, freed on remove/destroy).
// Values are void* — caller manages their lifetime.

#ifndef TIFERET_UTILS_MAP_H
#define TIFERET_UTILS_MAP_H

#include <stdbool.h>
#include <stddef.h>

// *** types

// ** type: tif_map_entry
/// A single key-value slot in the hash map.
typedef struct {
    char* key;          // Owned key string (NULL = empty slot)
    void* value;        // Caller-managed value pointer
} TifMapEntry;

// ** type: tif_map
/// A string-keyed hash map with open addressing.
typedef struct {
    TifMapEntry* entries;   // Array of slots
    size_t capacity;        // Total allocated slots
    size_t count;           // Number of occupied slots
} TifMap;

// *** functions

/// Initialize an empty map.
void tif_map_init(TifMap* map);

/// Destroy a map, freeing all owned keys.
/// Does NOT free the pointed-to values.
void tif_map_destroy(TifMap* map);

/// Set a key-value pair. The key is copied.
/// If the key already exists, the value is replaced.
/// Returns false on allocation failure.
bool tif_map_set(TifMap* map, const char* key, void* value);

/// Get the value for a key. Returns NULL if not found.
void* tif_map_get(const TifMap* map, const char* key);

/// Check whether a key exists in the map.
bool tif_map_has(const TifMap* map, const char* key);

/// Remove a key-value pair. Frees the key.
/// Returns the removed value, or NULL if not found.
void* tif_map_remove(TifMap* map, const char* key);

/// Return the number of key-value pairs.
size_t tif_map_size(const TifMap* map);

/// Return true if the map is empty.
bool tif_map_is_empty(const TifMap* map);

/// Clear all entries, freeing all keys.
/// Does NOT free the pointed-to values.
void tif_map_clear(TifMap* map);

// *** iteration

// ** type: tif_map_iter
/// Iterator state for traversing map entries.
typedef struct {
    const TifMap* map;  // Map being iterated
    size_t index;       // Current slot index
} TifMapIter;

/// Initialize an iterator for the given map.
void tif_map_iter_init(TifMapIter* iter, const TifMap* map);

/// Advance to the next entry. Returns false when exhausted.
/// On success, *key and *value point to the current entry's data.
bool tif_map_iter_next(TifMapIter* iter, const char** key, void** value);

#endif // TIFERET_UTILS_MAP_H
