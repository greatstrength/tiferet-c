// *** examples/calculator/main.c
// A simple calculator demonstrating the Tiferet C framework.
//
// This example shows the complete vertical for an in-memory feature pipeline:
//   manual configs → TifServiceRegistry → TifDIContainer →
//   TifAppContext → feature pipeline execution.
//
// NOTE: YAML repositories are not yet implemented in the C framework.
//       Features, errors, and service configurations are wired programmatically
//       to mirror the structure defined in examples/calculator/configs/.
//
// Build:
//   cmake -S ../.. -B build -DTIFERET_BUILD_EXAMPLES=ON
//   make -C build calculator
//
// Run (from the project root):
//   ./build/examples/calculator/calculator

#include <tiferet/tiferet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// *** helpers

// ** helper: get_value
/// Look up a numeric value by key: params first, then data.
/// Returns 0.0 if the key is not present in either map.
static double get_value(const TifMap* params,
                        const TifMap* data,
                        const char* key) {
    const char* s = (const char*)tif_map_get(params, key);
    if (!s) s = (const char*)tif_map_get(data, key);
    if (!s) return 0.0;
    return strtod(s, NULL);
}

// ** helper: format_result
/// Allocate a heap string for a double value using %g formatting.
/// Returns NULL on allocation failure. Caller owns the result.
static char* format_result(double val) {
    char* buf = (char*)malloc(64);
    if (!buf) return NULL;
    snprintf(buf, 64, "%g", val);
    return buf;
}

// *** events

// ** event: add_calc_event
/// Reads "a" and "b" (params then data), returns a + b as a string.

typedef struct { TifExecutableEvent vtable; } AddCalcEvent;

static void add_event_destroy(TifService* self) { free(self); }

static TifResult add_event_execute(TifExecutableEvent* self,
                                   const TifMap* data,
                                   const TifMap* params,
                                   char** result_out) {
    (void)self;
    double a = get_value(params, data, "a");
    double b = get_value(params, data, "b");
    *result_out = format_result(a + b);
    return TIF_OK;
}

static TifService* add_event_factory(const TifMap* params) {
    (void)params;
    AddCalcEvent* evt = (AddCalcEvent*)malloc(sizeof(AddCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = add_event_destroy;
    evt->vtable.execute      = add_event_execute;
    return (TifService*)evt;
}

// ** event: subtract_calc_event
/// Reads "a" and "b" (params then data), returns a - b as a string.

typedef struct { TifExecutableEvent vtable; } SubtractCalcEvent;

static void subtract_event_destroy(TifService* self) { free(self); }

static TifResult subtract_event_execute(TifExecutableEvent* self,
                                        const TifMap* data,
                                        const TifMap* params,
                                        char** result_out) {
    (void)self;
    double a = get_value(params, data, "a");
    double b = get_value(params, data, "b");
    *result_out = format_result(a - b);
    return TIF_OK;
}

static TifService* subtract_event_factory(const TifMap* params) {
    (void)params;
    SubtractCalcEvent* evt =
        (SubtractCalcEvent*)malloc(sizeof(SubtractCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = subtract_event_destroy;
    evt->vtable.execute      = subtract_event_execute;
    return (TifService*)evt;
}

// ** event: multiply_calc_event
/// Reads "a" and "b" (params then data), returns a * b as a string.

typedef struct { TifExecutableEvent vtable; } MultiplyCalcEvent;

static void multiply_event_destroy(TifService* self) { free(self); }

static TifResult multiply_event_execute(TifExecutableEvent* self,
                                        const TifMap* data,
                                        const TifMap* params,
                                        char** result_out) {
    (void)self;
    double a = get_value(params, data, "a");
    double b = get_value(params, data, "b");
    *result_out = format_result(a * b);
    return TIF_OK;
}

static TifService* multiply_event_factory(const TifMap* params) {
    (void)params;
    MultiplyCalcEvent* evt =
        (MultiplyCalcEvent*)malloc(sizeof(MultiplyCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = multiply_event_destroy;
    evt->vtable.execute      = multiply_event_execute;
    return (TifService*)evt;
}

// ** event: divide_calc_event
/// Reads "a" and "b" (params then data), returns a / b as a string.
/// Returns TIF_ERR_DIVISION_BY_ZERO if b is zero.

typedef struct { TifExecutableEvent vtable; } DivideCalcEvent;

static void divide_event_destroy(TifService* self) { free(self); }

static TifResult divide_event_execute(TifExecutableEvent* self,
                                      const TifMap* data,
                                      const TifMap* params,
                                      char** result_out) {
    (void)self;
    double a = get_value(params, data, "a");
    double b = get_value(params, data, "b");
    TIF_VERIFY(b != 0.0, TIF_ERR_DIVISION_BY_ZERO, "Cannot divide by zero");
    *result_out = format_result(a / b);
    return TIF_OK;
}

static TifService* divide_event_factory(const TifMap* params) {
    (void)params;
    DivideCalcEvent* evt = (DivideCalcEvent*)malloc(sizeof(DivideCalcEvent));
    if (!evt) return NULL;
    evt->vtable.base.destroy = divide_event_destroy;
    evt->vtable.execute      = divide_event_execute;
    return (TifService*)evt;
}

// *** services

// ** service: calc_feature_service
/// In-memory TifFeatureService backed by a stack-allocated features array.

typedef struct {
    TifFeatureService vtable;
    TifFeature*       features;
    size_t            count;
} CalcFeatureService;

static const TifFeature* calc_feature_get(TifFeatureService* self,
                                          const char* id) {
    CalcFeatureService* svc = (CalcFeatureService*)self;
    for (size_t i = 0; i < svc->count; ++i) {
        if (svc->features[i].id &&
            strcmp(svc->features[i].id, id) == 0) {
            return &svc->features[i];
        }
    }
    return NULL;
}

static bool calc_feature_exists(TifFeatureService* self, const char* id) {
    return calc_feature_get(self, id) != NULL;
}

static void calc_feature_service_destroy(TifService* self) {
    (void)self; // Stack-allocated — nothing to free.
}

// ** service: calc_error_service
/// In-memory TifErrorService backed by a stack-allocated errors array.

typedef struct {
    TifErrorService vtable;
    TifError*       errors;
    size_t          count;
} CalcErrorService;

static const TifError* calc_error_get(TifErrorService* self,
                                      const char* id_or_code) {
    CalcErrorService* svc = (CalcErrorService*)self;
    for (size_t i = 0; i < svc->count; ++i) {
        if ((svc->errors[i].id &&
             strcmp(svc->errors[i].id, id_or_code) == 0) ||
            (svc->errors[i].error_code &&
             strcmp(svc->errors[i].error_code, id_or_code) == 0)) {
            return &svc->errors[i];
        }
    }
    return NULL;
}

static bool calc_error_exists(TifErrorService* self, const char* id_or_code) {
    return calc_error_get(self, id_or_code) != NULL;
}

static void calc_error_service_destroy(TifService* self) {
    (void)self; // Stack-allocated — nothing to free.
}

// *** main

int main(void) {

    // --- Build features (mirrors examples/calculator/configs/feature.yml) ---

    TifFeature features[4];

    // calc.add
    tif_feature_init(&features[0], "calc.add", "Add Number",
                     "calc", "add", "Adds one number to another");
    {
        TifFeatureEvent step;
        tif_feature_event_init(&step, "Add a and b",
                               "add_event", "result", false, NULL);
        tif_feature_add_step(&features[0], step); // ownership transferred
    }

    // calc.subtract
    tif_feature_init(&features[1], "calc.subtract", "Subtract Number",
                     "calc", "subtract", "Subtracts one number from another");
    {
        TifFeatureEvent step;
        tif_feature_event_init(&step, "Subtract b from a",
                               "subtract_event", "result", false, NULL);
        tif_feature_add_step(&features[1], step);
    }

    // calc.multiply
    tif_feature_init(&features[2], "calc.multiply", "Multiply Number",
                     "calc", "multiply", "Multiplies one number by another");
    {
        TifFeatureEvent step;
        tif_feature_event_init(&step, "Multiply a and b",
                               "multiply_event", "result", false, NULL);
        tif_feature_add_step(&features[2], step);
    }

    // calc.divide
    tif_feature_init(&features[3], "calc.divide", "Divide Number",
                     "calc", "divide", "Divides one number by another");
    {
        TifFeatureEvent step;
        tif_feature_event_init(&step, "Divide a by b",
                               "divide_event", "result", false, NULL);
        tif_feature_add_step(&features[3], step);
    }

    // --- Build errors (mirrors examples/calculator/configs/error.yml) ---

    TifError errors[1];
    tif_error_init(&errors[0], "division_by_zero",
                   "Division By Zero", NULL, NULL);
    tif_error_add_message(&errors[0], "en_US", "Cannot divide by zero");

    // --- Set up in-memory services ---

    CalcFeatureService feature_svc;
    feature_svc.vtable.base.destroy = calc_feature_service_destroy;
    feature_svc.vtable.get          = calc_feature_get;
    feature_svc.vtable.exists       = calc_feature_exists;
    feature_svc.features            = features;
    feature_svc.count               = 4;

    CalcErrorService error_svc;
    error_svc.vtable.base.destroy = calc_error_service_destroy;
    error_svc.vtable.get          = calc_error_get;
    error_svc.vtable.exists       = calc_error_exists;
    error_svc.errors              = errors;
    error_svc.count               = 1;

    // --- Build service configurations (mirrors configs/container.yml) ---

    TifServiceConfiguration configs[4];
    tif_service_config_init(&configs[0], "add_event",
                            NULL, "calc.events", "AddEvent");
    tif_service_config_init(&configs[1], "subtract_event",
                            NULL, "calc.events", "SubtractEvent");
    tif_service_config_init(&configs[2], "multiply_event",
                            NULL, "calc.events", "MultiplyEvent");
    tif_service_config_init(&configs[3], "divide_event",
                            NULL, "calc.events", "DivideEvent");

    // --- Set up the service registry ---

    TifServiceRegistry registry;
    tif_registry_init(&registry);
    tif_registry_register(&registry, "calc.events::AddEvent",
                          add_event_factory);
    tif_registry_register(&registry, "calc.events::SubtractEvent",
                          subtract_event_factory);
    tif_registry_register(&registry, "calc.events::MultiplyEvent",
                          multiply_event_factory);
    tif_registry_register(&registry, "calc.events::DivideEvent",
                          divide_event_factory);

    // --- Build the DI container ---

    TifDIContainer container;
    tif_di_init(&container, configs, 4, NULL, 0, &registry);

    // --- Create the application context ---

    TifAppContext app;
    tif_app_ctx_init(&app,
                     (TifFeatureService*)&feature_svc,
                     (TifErrorService*)&error_svc,
                     &container);

    // --- Define test cases ---

    struct {
        const char* feature_id;
        const char* a;
        const char* b;
        const char* op;
    } cases[] = {
        {"calc.add",      "1", "2", "+"},
        {"calc.subtract", "5", "3", "-"},
        {"calc.multiply", "4", "3", "*"},
        {"calc.divide",   "8", "2", "/"},
        {"calc.divide",   "8", "0", "/"},   // division by zero — expect error
    };
    size_t case_count = sizeof(cases) / sizeof(cases[0]);

    // --- Execute each test case ---

    for (size_t i = 0; i < case_count; ++i) {

        // Build the input data map for this test case.
        TifMap data;
        tif_map_init(&data);
        tif_map_set(&data, "a", (void*)cases[i].a);
        tif_map_set(&data, "b", (void*)cases[i].b);

        // Run the feature pipeline.
        TifResult r = tif_app_ctx_run(&app, cases[i].feature_id, &data);

        if (TIF_IS_OK(r)) {

            // Print the result stored under "result".
            char* result = (char*)tif_map_get(&data, "result");
            printf("%s %s %s = %s\n",
                   cases[i].a, cases[i].op, cases[i].b,
                   result ? result : "(null)");

            // Free the heap-allocated result string stored by the event.
            free(result);

        } else {

            // Look up the error message via the error context.
            char msg[256];
            int n = tif_error_ctx_format(tif_app_ctx_error_ctx(&app),
                                         r.error_code, "en_US",
                                         msg, sizeof(msg));

            // Fall back to the inline message or the error code if not found.
            if (n <= 0) {
                if (r.message) {
                    snprintf(msg, sizeof(msg), "%s", r.message);
                } else {
                    snprintf(msg, sizeof(msg), "%s",
                             r.error_code ? r.error_code : "Unknown error");
                }
            }
            printf("Error: %s\n", msg);
        }

        // Destroy the data map (frees heap-copied keys; "a"/"b" values are
        // literals and need no free; "result" was already freed above).
        tif_map_destroy(&data);
    }

    // --- Clean up ---

    tif_di_destroy(&container);
    tif_registry_destroy(&registry);

    for (size_t i = 0; i < 4; ++i) {
        tif_service_config_destroy(&configs[i]);
        tif_feature_destroy(&features[i]);
    }
    tif_error_destroy(&errors[0]);

    return 0;
}
