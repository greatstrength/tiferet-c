// *** src/utils/map.c
// Implementation of the TifMap string-keyed hash map.
// Uses open addressing with linear probing and a load factor of 0.75.

#include <tiferet/utils/map.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** constants

// ** constant: initial capacity
#define TIF_MAP_INITIAL_CAPACITY 16

// ** constant: load factor threshold (75%)
#define TIF_MAP_LOAD_FACTOR_NUM 3
#define TIF_MAP_LOAD_FACTOR_DEN 4

// ** constant: tombstone marker
// A removed slot uses this sentinel key to maintain probe chains.
static char TIF_MAP_TOMBSTONE_SENTINEL = '\0';
#define TIF_MAP_TOMBSTONE (&TIF_MAP_TOMBSTONE_SENTINEL)

// *** internal functions

// ** function: hash_string
/// FNV-1a hash for a NUL-terminated string.
static size_t hash_string(const char* key) {
    size_t hash = 14695981039346656037ULL;
    for (const char* p = key; *p; ++p) {
        hash ^= (size_t)(unsigned char)*p;
        hash *= 1099511628211ULL;
    }
    return hash;
}

// ** function: find_slot
/// Find the slot for a key. Returns the index.
/// If the key exists, the slot contains the matching entry.
/// If not, the slot is the first empty or tombstone slot for insertion.
static size_t find_slot(const TifMapEntry* entries, size_t capacity,
                        const char* key) {

    size_t idx = hash_string(key) & (capacity - 1);

    for (;;) {
        const char* slot_key = entries[idx].key;

        // Empty slot — key not found; this is the insertion point.
        if (!slot_key) return idx;

        // Skip tombstones during search.
        if (slot_key != TIF_MAP_TOMBSTONE && strcmp(slot_key, key) == 0) {
            return idx;
        }

        // Linear probe.
        idx = (idx + 1) & (capacity - 1);
    }
}

// ** function: resize
/// Grow the map to the given new capacity, rehashing all entries.
static bool resize(TifMap* map, size_t new_capacity) {

    // Allocate new entries array (zero-initialized = all empty).
    TifMapEntry* new_entries = (TifMapEntry*)calloc(
        new_capacity, sizeof(TifMapEntry));
    if (!new_entries) return false;

    // Rehash existing entries into the new array.
    for (size_t i = 0; i < map->capacity; ++i) {
        TifMapEntry* e = &map->entries[i];
        if (e->key && e->key != TIF_MAP_TOMBSTONE) {
            size_t idx = find_slot(new_entries, new_capacity, e->key);
            new_entries[idx] = *e;
        }
    }

    // Replace the old array.
    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;

    return true;
}

// *** functions

// ** function: tif_map_init
void tif_map_init(TifMap* map) {
    map->entries = NULL;
    map->capacity = 0;
    map->count = 0;
}

// ** function: tif_map_destroy
void tif_map_destroy(TifMap* map) {

    // Free all owned keys.
    for (size_t i = 0; i < map->capacity; ++i) {
        if (map->entries[i].key && map->entries[i].key != TIF_MAP_TOMBSTONE) {
            free(map->entries[i].key);
        }
    }

    // Free the entries array.
    free(map->entries);

    // Reset fields.
    map->entries = NULL;
    map->capacity = 0;
    map->count = 0;
}

// ** function: tif_map_set
bool tif_map_set(TifMap* map, const char* key, void* value) {

    // Handle NULL key.
    if (!key) return false;

    // Ensure capacity exists and check load factor.
    if (map->capacity == 0) {
        if (!resize(map, TIF_MAP_INITIAL_CAPACITY)) return false;
    } else if (map->count * TIF_MAP_LOAD_FACTOR_DEN >=
               map->capacity * TIF_MAP_LOAD_FACTOR_NUM) {
        if (!resize(map, map->capacity * 2)) return false;
    }

    // Find the slot.
    size_t idx = find_slot(map->entries, map->capacity, key);
    TifMapEntry* entry = &map->entries[idx];

    // If the key already exists, update the value.
    if (entry->key && entry->key != TIF_MAP_TOMBSTONE) {
        entry->value = value;
        return true;
    }

    // Insert new entry — copy the key.
    entry->key = tif_strdup(key);
    if (!entry->key) return false;

    entry->value = value;
    map->count++;

    return true;
}

// ** function: tif_map_get
void* tif_map_get(const TifMap* map, const char* key) {

    // Handle empty map or NULL key.
    if (!key || map->capacity == 0) return NULL;

    // Find the slot.
    size_t idx = find_slot(map->entries, map->capacity, key);
    TifMapEntry* entry = (TifMapEntry*)&map->entries[idx];

    // Return the value if found.
    if (entry->key && entry->key != TIF_MAP_TOMBSTONE) {
        return entry->value;
    }

    return NULL;
}

// ** function: tif_map_has
bool tif_map_has(const TifMap* map, const char* key) {

    // Handle empty map or NULL key.
    if (!key || map->capacity == 0) return false;

    // Find the slot.
    size_t idx = find_slot(map->entries, map->capacity, key);

    return map->entries[idx].key != NULL &&
           map->entries[idx].key != TIF_MAP_TOMBSTONE;
}

// ** function: tif_map_remove
void* tif_map_remove(TifMap* map, const char* key) {

    // Handle empty map or NULL key.
    if (!key || map->capacity == 0) return NULL;

    // Find the slot.
    size_t idx = find_slot(map->entries, map->capacity, key);
    TifMapEntry* entry = &map->entries[idx];

    // If not found, return NULL.
    if (!entry->key || entry->key == TIF_MAP_TOMBSTONE) return NULL;

    // Save the value, free the key, mark as tombstone.
    void* value = entry->value;
    free(entry->key);
    entry->key = TIF_MAP_TOMBSTONE;
    entry->value = NULL;
    map->count--;

    return value;
}

// ** function: tif_map_size
size_t tif_map_size(const TifMap* map) {
    return map->count;
}

// ** function: tif_map_is_empty
bool tif_map_is_empty(const TifMap* map) {
    return map->count == 0;
}

// ** function: tif_map_clear
void tif_map_clear(TifMap* map) {

    // Free all owned keys and reset entries.
    for (size_t i = 0; i < map->capacity; ++i) {
        if (map->entries[i].key && map->entries[i].key != TIF_MAP_TOMBSTONE) {
            free(map->entries[i].key);
        }
        map->entries[i].key = NULL;
        map->entries[i].value = NULL;
    }

    map->count = 0;
}

// *** iteration

// ** function: tif_map_iter_init
void tif_map_iter_init(TifMapIter* iter, const TifMap* map) {
    iter->map = map;
    iter->index = 0;
}

// ** function: tif_map_iter_next
bool tif_map_iter_next(TifMapIter* iter, const char** key, void** value) {

    // Scan forward for the next occupied slot.
    while (iter->index < iter->map->capacity) {
        const TifMapEntry* entry = &iter->map->entries[iter->index];
        iter->index++;

        if (entry->key && entry->key != TIF_MAP_TOMBSTONE) {
            *key = entry->key;
            *value = entry->value;
            return true;
        }
    }

    return false;
}
