// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Xilinx, Inc
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

// ------ I N C L U D E   F I L E S -------------------------------------------
// Local - Include Files
#include "core/common/device.h"
#include "core/common/query_requests.h"
#include "OO_AieClockFreq.h"
#include "tools/common/XBUtilitiesCore.h"
#include "tools/common/XBUtilities.h"

namespace XBU = XBUtilities;

// 3rd Party Library - Include Files
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "core/common/aie_freq_utils.h"

namespace po = boost::program_options;
namespace qr = xrt_core::query;

// System - Include Files
#include <iostream>

// ----- C L A S S   M E T H O D S -------------------------------------------
OO_AieClockFreq::OO_AieClockFreq( const std::string &_longName, bool _isHidden )
    : OptionOptions(_longName, _isHidden, "AIE clock frequency operations" )
    , m_device("")
    , m_partition_id(1)
    , m_get(false)
    , m_setFreq("")
    , m_help(false)
{
  m_optionsDescription.add_options()
    ("device,d", po::value<decltype(m_device)>(&m_device), "The Bus:Device.Function (e.g., 0000:d8:00.0) device of interest")
    ("partition,p", po::value<decltype(m_partition_id)>(&m_partition_id), "The Partition id of AIE")
    ("set,s", po::value<decltype(m_setFreq)>(&m_setFreq), "Frequency value (Hz) to set given AIE partition to (eg: 100K, 312.5M, 5G)")
    ("get,g", po::bool_switch(&m_get), "Read the frequency of given AIE partition")
    ("help,h", po::bool_switch(&m_help), "Help to use this sub-command")
  ;
}

void
OO_AieClockFreq::execute(const SubCmdOptions& _options) const
{
  XBU::verbose("SubCommand option: AIE Clock");

  XBU::verbose("Option(s):");
  for (auto & aString : _options)
    XBU::verbose(std::string(" ") + aString);

  // Honor help option first
  if (std::find(_options.begin(), _options.end(), "--help") != _options.end()) {
    printHelp();
    return;
  }

  // Parse sub-command ...
  po::variables_map vm;
  process_arguments(vm, _options);

  // Exit if action is specified
  if(m_help) {
    printHelp();
    return;
  }

  // Check if set/get is used
  if(!m_get && m_setFreq.length() == 0) {
    std::cerr << "ERROR: Missing 'set' or 'get' option" << std::endl;
    std::cerr << "please use any one of set/get and rerun" << std::endl;
    printHelp();
    throw xrt_core::error(std::errc::operation_canceled);
  }

  // Check if partition_id is provided else print Warning!
  if(!vm.count("partition"))
      std::cout << "WARNING: 'partition' option is not provided, using default partition id value '1'" << std::endl;

  // Find device of interest
  std::shared_ptr<xrt_core::device> device;

  try {
    device = XBU::get_device(boost::algorithm::to_lower_copy(m_device), true /*inUserDomain*/);
  } catch (const std::runtime_error& e) {
    // Catch only the exceptions that we have generated earlier
    std::cerr << boost::format("ERROR: %s\n") % e.what();
    throw xrt_core::error(std::errc::operation_canceled);
  }

  // Do operations on the device collected
  if(m_get) {
    double freq_part = xrt_core::aie_freq::get_aie_part_freq(device, m_partition_id);
    std::cout << boost::format("INFO: Clock frequency of AIE partition(%d) is: %.2f MHz\n") % m_partition_id % freq_part ;
    return;
  }

  if(!m_setFreq.empty()) {

    // Display frequency before setting
    std::cout << boost::format("INFO: Clock frequency of AIE partition(%d) before setting is: %.2f MHz\n")
                 % m_partition_id % xrt_core::aie_freq::get_aie_part_freq(device, m_partition_id);
    bool status = xrt_core::aie_freq::set_aie_part_freq(device, m_partition_id, m_setFreq);

    if(status) {
      std::cout << boost::format("INFO: Setting clock freq of AIE partition(%d) is successful\n") % m_partition_id;
      std::cout << boost::format("Running clock freq of AIE partition(%d) is: %.2f MHz\n")
                   % m_partition_id % xrt_core::aie_freq::get_aie_part_freq(device, m_partition_id);
    }
    else {
      throw std::runtime_error("AIE driver call to set freq failed");
    }
  }
}
