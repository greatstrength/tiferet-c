// *** src/events/settings.c
// Implementation of domain event helper functions.

#include <tiferet/events/settings.h>

// *** functions

// ** function: tif_raise_error
TifResult tif_raise_error(const char* error_code, const char* message) {
    return TIF_ERR_MSG(error_code, message);
}
