// *** tiferet/assets/constants.h
// Error code string constants for the Tiferet C framework.
// Mirrors error codes from Python (assets/constants.py),
// C# (Domain/ErrorCodes.cs), and C++ (assets/constants.h).

#ifndef TIFERET_ASSETS_CONSTANTS_H
#define TIFERET_ASSETS_CONSTANTS_H

// *** error codes

// ** error: parameter validation
#define TIF_ERR_COMMAND_PARAMETER_REQUIRED   "COMMAND_PARAMETER_REQUIRED"
#define TIF_ERR_PARAMETER_NOT_FOUND          "PARAMETER_NOT_FOUND"

// ** error: model validation
#define TIF_ERR_INVALID_MODEL_ATTRIBUTE      "INVALID_MODEL_ATTRIBUTE"

// ** error: feature errors
#define TIF_ERR_FEATURE_NOT_FOUND            "FEATURE_NOT_FOUND"
#define TIF_ERR_FEATURE_ALREADY_EXISTS       "FEATURE_ALREADY_EXISTS"
#define TIF_ERR_FEATURE_COMMAND_LOADING_FAILED "FEATURE_COMMAND_LOADING_FAILED"

// ** error: error domain
#define TIF_ERR_ERROR_NOT_FOUND              "ERROR_NOT_FOUND"
#define TIF_ERR_ERROR_ALREADY_EXISTS         "ERROR_ALREADY_EXISTS"

// ** error: dependency injection
#define TIF_ERR_DEPENDENCY_TYPE_NOT_FOUND    "DEPENDENCY_TYPE_NOT_FOUND"
#define TIF_ERR_SERVICE_NOT_REGISTERED       "SERVICE_NOT_REGISTERED"
#define TIF_ERR_SERVICE_RESOLUTION_FAILED    "SERVICE_RESOLUTION_FAILED"

// ** error: app
#define TIF_ERR_APP_ERROR                    "APP_ERROR"
#define TIF_ERR_APP_INTERFACE_NOT_FOUND      "APP_INTERFACE_NOT_FOUND"
#define TIF_ERR_INVALID_APP_INTERFACE_TYPE   "INVALID_APP_INTERFACE_TYPE"

// ** error: request
#define TIF_ERR_REQUEST_NOT_FOUND            "REQUEST_NOT_FOUND"

// ** error: feature step execution
#define TIF_ERR_FEATURE_STEP_EXECUTION_FAILED "FEATURE_STEP_EXECUTION_FAILED"

// ** error: file / utility
#define TIF_ERR_INVALID_FILE_MODE            "INVALID_FILE_MODE"
#define TIF_ERR_INVALID_FILE_ENCODING        "INVALID_FILE_ENCODING"
#define TIF_ERR_FILE_NOT_FOUND               "FILE_NOT_FOUND"
#define TIF_ERR_INVALID_FILE                 "INVALID_FILE"

// ** error: calculator / general
#define TIF_ERR_DIVISION_BY_ZERO             "DIVISION_BY_ZERO"
#define TIF_ERR_INVALID_INPUT                "INVALID_INPUT"

// ** error: yaml
#define TIF_ERR_YAML_FILE_NOT_FOUND          "YAML_FILE_NOT_FOUND"
#define TIF_ERR_YAML_LOAD_ERROR              "YAML_LOAD_ERROR"
#define TIF_ERR_YAML_SAVE_ERROR              "YAML_SAVE_ERROR"

#endif // TIFERET_ASSETS_CONSTANTS_H
