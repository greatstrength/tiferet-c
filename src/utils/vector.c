// *** src/utils/vector.c
// Implementation of the TifVector dynamic array.

#include <tiferet/utils/vector.h>

#include <stdlib.h>
#include <string.h>

// *** constants

// ** constant: initial capacity
#define TIF_VECTOR_INITIAL_CAPACITY 8

// *** functions

// ** function: tif_vector_init
void tif_vector_init(TifVector* vec) {

    // Zero-initialize all fields.
    vec->items = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

// ** function: tif_vector_destroy
void tif_vector_destroy(TifVector* vec) {

    // Free the internal storage.
    free(vec->items);

    // Reset fields to prevent use-after-free.
    vec->items = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

// ** function: tif_vector_push
bool tif_vector_push(TifVector* vec, void* item) {

    // Grow if at capacity.
    if (vec->count >= vec->capacity) {
        size_t new_cap = vec->capacity == 0
            ? TIF_VECTOR_INITIAL_CAPACITY
            : vec->capacity * 2;

        void** new_items = (void**)realloc(vec->items, new_cap * sizeof(void*));
        if (!new_items) return false;

        vec->items = new_items;
        vec->capacity = new_cap;
    }

    // Append the item.
    vec->items[vec->count] = item;
    vec->count++;

    return true;
}

// ** function: tif_vector_get
void* tif_vector_get(const TifVector* vec, size_t index) {

    // Bounds check.
    if (index >= vec->count) return NULL;

    return vec->items[index];
}

// ** function: tif_vector_remove
void* tif_vector_remove(TifVector* vec, size_t index) {

    // Bounds check.
    if (index >= vec->count) return NULL;

    // Save the element being removed.
    void* removed = vec->items[index];

    // Shift subsequent elements left.
    size_t remaining = vec->count - index - 1;
    if (remaining > 0) {
        memmove(&vec->items[index], &vec->items[index + 1],
                remaining * sizeof(void*));
    }

    vec->count--;
    return removed;
}

// ** function: tif_vector_size
size_t tif_vector_size(const TifVector* vec) {
    return vec->count;
}

// ** function: tif_vector_is_empty
bool tif_vector_is_empty(const TifVector* vec) {
    return vec->count == 0;
}

// ** function: tif_vector_clear
void tif_vector_clear(TifVector* vec) {
    vec->count = 0;
}
