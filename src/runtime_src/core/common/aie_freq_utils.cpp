#include "aie_freq_utils.h"
#include "core/common/query_requests.h"
#include "core/common/error.h"
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace qr = xrt_core::query;

namespace xrt_core { namespace aie_freq {

static double to_megaHz(uint64_t freq_hz) {
  return static_cast<double>(freq_hz) / 1000000.0;
}

static uint64_t string_to_freq_units(const std::string& input)
{
    if (input.empty())
        throw xrt_core::error(std::errc::invalid_argument, "Empty frequency string");

    constexpr uint64_t factor = 1000;
    std::string str = input;

    char unit = 'B';
    if (std::isalpha(str.back())) {
        unit = std::toupper(str.back());
        str.pop_back();
    }

    uint64_t size = 0;
    try {
        size = std::stoull(str);
    } catch (const std::exception&) {
        throw xrt_core::error(std::errc::invalid_argument, "Invalid numeric value: " + str);
    }

    uint64_t multiplier;
    switch (unit) {
        case 'B': multiplier = 1; break;                // Hz
        case 'K': multiplier = factor; break;           // KHz
        case 'M': multiplier = factor * factor; break;  // MHz
        case 'G': multiplier = factor * factor * factor; break; // GHz
        default:
            throw xrt_core::error(std::errc::invalid_argument,
                "Invalid frequency unit: " + std::string(1, unit) + ". Use B, K, M, or G");
    }

    return size * multiplier;
}

double get_aie_part_freq(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id)
{
  try {
    return to_megaHz(xrt_core::device_query<qr::aie_get_freq>(device, part_id));
  }
  catch (const xrt_core::query::no_such_key&) {
    throw std::runtime_error("get_aie_freq is not supported on this platform");
  }
  catch (const std::exception &e) {
    throw std::runtime_error("Failed to read clock frequency of AIE partition(" + 
                           std::to_string(part_id) + "): " + e.what());
  }
}

bool
set_aie_part_freq_hz(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id, uint64_t freq_hz)
{
  try {
    std::cout<<"displaying freq_in hertz: "<<freq_hz;
    return xrt_core::device_query<qr::aie_set_freq>(device, part_id, freq_hz);
  }
  catch (const xrt_core::query::no_such_key&) {
    throw std::runtime_error("set_aie_freq is not supported on this platform");
  }
  catch (const std::exception& e) {
    throw std::runtime_error("Failed to set AIE partition frequency: " + std::string(e.what()));
  }
}

bool
set_aie_part_freq(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id, const std::string& freq_str)
{
  uint64_t freq_hz = 0;
  try {
    freq_hz = string_to_freq_units(freq_str);
  }
  catch(const xrt_core::error&) {
    throw std::runtime_error("Invalid frequency format: " + freq_str);
  }
  return set_aie_part_freq_hz(device, part_id, freq_hz);
}

}} // namespace aie_freq, xrt_core
