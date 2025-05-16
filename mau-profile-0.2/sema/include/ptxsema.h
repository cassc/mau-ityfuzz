#pragma once

#ifdef _MSC_VER

#define EXPORT
#endif

#else
#define EXPORT __attribute__ ((visibility ("default")))
#endif

#if __cplusplus
extern "C" {
#endif

/// Create PTXSema instance.
///
/// @return  The PTXSema instance.
// EXPORT struct evmc_instance* evmtrans_create(void);
// EVMC_EXPORT struct evmc_vm* evmc_create_evmtrans(void) EVMC_NOEXCEPT;
// EVMC_EXPORT struct evmc_instance* evmc_create_evmtrans(void) EVMC_NOEXCEPT;

#if __cplusplus
}
#endif
