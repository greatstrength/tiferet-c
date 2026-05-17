// *** tiferet/interfaces/settings.h
// Base service interface for the Tiferet C framework.
// TifService is a vtable struct with a destroy function pointer.
// All domain-specific service interfaces embed TifService as their
// first member, enabling safe casting between base and derived types.
//
// Mirrors:
//   Python: Service(ABC)
//   C#:     IService
//   C++:    class Service { virtual ~Service() = default; }

#ifndef TIFERET_INTERFACES_SETTINGS_H
#define TIFERET_INTERFACES_SETTINGS_H

// *** interfaces

// ** interface: tif_service
/// Abstract base for all service contracts.
/// Concrete implementations must set the destroy function pointer
/// to properly clean up their resources.
typedef struct TifService {

    /// Destroy this service instance, freeing all owned resources.
    /// Implementations must also free the struct itself if heap-allocated.
    void (*destroy)(struct TifService* self);

} TifService;

#endif // TIFERET_INTERFACES_SETTINGS_H
