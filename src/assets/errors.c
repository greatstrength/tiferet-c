// *** src/assets/errors.c
// Implementation of TifResult helper functions.

#include <tiferet/assets/errors.h>

#include <stdio.h>

// *** functions

// ** function: tif_result_format
int tif_result_format(const TifResult* result, char* buf, size_t buf_size) {

    // Handle NULL result pointer.
    if (!result || !buf || buf_size == 0) return 0;

    // Format success case.
    if (TIF_IS_OK(*result)) {
        return snprintf(buf, buf_size, "OK");
    }

    // Format error with message.
    if (result->message) {
        return snprintf(buf, buf_size, "%s: %s",
                        result->error_code, result->message);
    }

    // Format error without message.
    return snprintf(buf, buf_size, "%s", result->error_code);
}
