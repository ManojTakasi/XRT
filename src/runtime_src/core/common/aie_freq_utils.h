#ifndef AIE_FREQ_UTILS_H
#define AIE_FREQ_UTILS_H

#include "core/common/device.h"

namespace xrt_core { namespace aie_freq {

double get_aie_part_freq(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id);

bool
set_aie_part_freq(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id, const std::string& freq_str);

bool
set_aie_part_freq_hz(const std::shared_ptr<xrt_core::device>& device, uint32_t part_id, uint64_t freq_hz);

}} // namespace aie_freq, xrt_core

#endif
