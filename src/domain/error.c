// *** src/domain/error.c
// Implementation of TifErrorMessage and TifError domain functions.

#include <tiferet/domain/error.h>
#include <tiferet/utils/strutil.h>

#include <stdlib.h>
#include <string.h>

// *** functions: TifErrorMessage

// ** function: tif_error_message_init
TifResult tif_error_message_init(TifErrorMessage* msg,
                                 const char* lang,
                                 const char* text) {

    // Zero-initialize the struct.
    memset(msg, 0, sizeof(TifErrorMessage));

    // Copy lang.
    msg->lang = tif_strdup(lang);
    if (lang && !msg->lang) return TIF_ERR("ALLOC_FAILED");

    // Copy text.
    msg->text = tif_strdup(text);
    if (text && !msg->text) {
        free(msg->lang);
        msg->lang = NULL;
        return TIF_ERR("ALLOC_FAILED");
    }

    return TIF_OK;
}

// ** function: tif_error_message_destroy
void tif_error_message_destroy(TifErrorMessage* msg) {

    // Free owned strings.
    free(msg->lang);
    free(msg->text);

    // Reset to prevent use-after-free.
    msg->lang = NULL;
    msg->text = NULL;
}

// ** function: tif_error_message_validate
bool tif_error_message_validate(const TifErrorMessage* msg) {
    return !tif_str_is_empty(msg->lang) && !tif_str_is_empty(msg->text);
}

// *** functions: TifError

// ** function: tif_error_init
TifResult tif_error_init(TifError* err,
                         const char* id,
                         const char* name,
                         const char* description,
                         const char* error_code) {

    // Zero-initialize the struct.
    memset(err, 0, sizeof(TifError));

    // Copy id.
    err->id = tif_strdup(id);
    if (id && !err->id) return TIF_ERR("ALLOC_FAILED");

    // Copy name.
    err->name = tif_strdup(name);
    if (name && !err->name) {
        tif_error_destroy(err);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Copy description (optional).
    if (description) {
        err->description = tif_strdup(description);
        if (!err->description) {
            tif_error_destroy(err);
            return TIF_ERR("ALLOC_FAILED");
        }
    }

    // Derive or copy error_code.
    if (error_code) {
        err->error_code = tif_strdup(error_code);
    } else if (id) {
        err->error_code = tif_error_derive_error_code(id);
    }
    if ((error_code || id) && !err->error_code) {
        tif_error_destroy(err);
        return TIF_ERR("ALLOC_FAILED");
    }

    // Messages start empty.
    err->messages = NULL;
    err->message_count = 0;

    return TIF_OK;
}

// ** function: tif_error_destroy
void tif_error_destroy(TifError* err) {

    // Free owned strings.
    free(err->id);
    free(err->name);
    free(err->description);
    free(err->error_code);

    // Free each message in the array.
    for (size_t i = 0; i < err->message_count; ++i) {
        tif_error_message_destroy(&err->messages[i]);
    }
    free(err->messages);

    // Reset all fields.
    memset(err, 0, sizeof(TifError));
}

// ** function: tif_error_validate
bool tif_error_validate(const TifError* err) {
    return !tif_str_is_empty(err->id) && !tif_str_is_empty(err->name);
}

// ** function: tif_error_add_message
TifResult tif_error_add_message(TifError* err,
                                const char* lang,
                                const char* text) {

    // Grow the messages array by one slot.
    size_t new_count = err->message_count + 1;
    TifErrorMessage* new_msgs = (TifErrorMessage*)realloc(
        err->messages, new_count * sizeof(TifErrorMessage));
    if (!new_msgs) return TIF_ERR("ALLOC_FAILED");

    err->messages = new_msgs;

    // Initialize the new message at the end.
    TifResult r = tif_error_message_init(&err->messages[err->message_count],
                                         lang, text);
    if (TIF_IS_ERR(r)) return r;

    err->message_count = new_count;

    return TIF_OK;
}

// ** function: tif_error_format_message
const char* tif_error_format_message(const TifError* err,
                                     const char* lang) {

    // Search for a matching language.
    for (size_t i = 0; i < err->message_count; ++i) {
        if (err->messages[i].lang && strcmp(err->messages[i].lang, lang) == 0) {
            return err->messages[i].text;
        }
    }

    return NULL;
}

// ** function: tif_error_derive_error_code
char* tif_error_derive_error_code(const char* id) {
    return tif_str_to_upper_code(id);
}
