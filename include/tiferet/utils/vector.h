// *** tiferet/utils/vector.h
// Type-erased dynamic array for the Tiferet C framework.
// Stores void* elements. Caller is responsible for the
// lifetime of pointed-to objects.

#ifndef TIFERET_UTILS_VECTOR_H
#define TIFERET_UTILS_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

// *** types

// ** type: tif_vector
/// A growable array of void* elements.
typedef struct {
    void** items;       // Array of element pointers
    size_t count;       // Number of elements
    size_t capacity;    // Allocated slots
} TifVector;

// *** functions

/// Initialize a vector with zero elements.
void tif_vector_init(TifVector* vec);

/// Destroy a vector, freeing its internal storage.
/// Does NOT free the pointed-to elements.
void tif_vector_destroy(TifVector* vec);

/// Append an element to the end of the vector.
/// Returns false on allocation failure.
bool tif_vector_push(TifVector* vec, void* item);

/// Get the element at the given index.
/// Returns NULL if index is out of range.
void* tif_vector_get(const TifVector* vec, size_t index);

/// Remove the element at the given index, shifting subsequent elements.
/// Returns the removed element, or NULL if index is out of range.
void* tif_vector_remove(TifVector* vec, size_t index);

/// Return the number of elements.
size_t tif_vector_size(const TifVector* vec);

/// Return true if the vector is empty.
bool tif_vector_is_empty(const TifVector* vec);

/// Clear all elements without freeing the internal buffer.
/// Does NOT free the pointed-to elements.
void tif_vector_clear(TifVector* vec);

#endif // TIFERET_UTILS_VECTOR_H
