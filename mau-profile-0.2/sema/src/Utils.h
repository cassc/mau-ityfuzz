#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

// metadata labels
static const auto c_EVMBB = "EVMBB";
static const auto c_pc = "pc";
static const auto c_destIdxLabel = "destIdx";
static const auto c_intsan = "intsan";
static const auto c_rensan = "rensan";
static const auto c_orgsan = "orgsan";
static const auto c_sucsan = "sucsan";
static const auto c_bsdsan = "bsdsan";
static const auto c_msesan = "msesan";
static const auto c_elksan = "elksan";

namespace dev {
namespace evmtrans {

typedef std::vector<uint8_t> Bytes;

void keccak(uint8_t const *_data, uint64_t _size, uint8_t *o_hash);


// The same as assert, but expression is always evaluated and result returned
#define CHECK(expr) (assert(expr), expr)

#if !defined(NDEBUG)  // Debug

std::ostream &getLogStream(char const *_channel);

#define DLOG(CHANNEL) ::dev::evmtrans::getLogStream(#CHANNEL)

#else  // Release

struct Voider {
  void operator=(std::ostream const &) {}
};

#define DLOG(CHANNEL) true ? (void)0 : ::dev::evmtrans::Voider{} = std::cerr

#endif

}  // namespace evmtrans
}  // namespace dev
