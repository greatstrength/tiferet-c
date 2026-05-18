// *** src/contexts/error.c
// Implementation of TifErrorContext.

#include <tiferet/contexts/error.h>

#include <stdio.h>
#include <string.h>

// *** functions

// ** function: tif_error_ctx_init
void tif_error_ctx_init(TifErrorContext* ctx,
                        TifErrorService* error_service) {

    // Store the borrowed error service reference.
    ctx->error_service = error_service;
}

// ** function: tif_error_ctx_get
const TifError* tif_error_ctx_get(const TifErrorContext* ctx,
                                  const char* id_or_code) {

    // Delegate to the error service.
    return ctx->error_service->get(ctx->error_service, id_or_code);
}

// ** function: tif_error_ctx_format
int tif_error_ctx_format(const TifErrorContext* ctx,
                         const char* id_or_code,
                         const char* lang,
                         char* buf,
                         size_t buf_size) {

    // Look up the error definition.
    const TifError* err =
        ctx->error_service->get(ctx->error_service, id_or_code);
    if (!err) return 0;

    // Use default language if not specified.
    const char* effective_lang = lang ? lang : "en_US";

    // Retrieve the localized message text.
    const char* text = tif_error_format_message(err, effective_lang);
    if (!text) return 0;

    // Copy the message into the caller-provided buffer.
    return snprintf(buf, buf_size, "%s", text);
}

// ** function: tif_error_ctx_exists
bool tif_error_ctx_exists(const TifErrorContext* ctx,
                          const char* id_or_code) {

    // Delegate to the error service.
    return ctx->error_service->exists(ctx->error_service, id_or_code);
}
