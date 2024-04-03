// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// USB streaming data test
//
// Linux host-side application that receives a stream of LFSR-generated data
// from the USB device, checks the received bytestream and then XORs it with a
// host-side LFSR-generated byte stream to transmit back to the device.
//
// By default the streaming test expects a number of USB serial connections to
// the target device, one port per endpoint:
//
//   /dev/ttyUSB0 - supplies and receives LFSR-generated byte stream for one/
//                  the only endpoint
//   /dev/ttyUSB1 - a secondary stream
//   /dev/ttyUSB..
//
// Note that the mapping from device endpoints to USB port number is not
// guaranteed, and  when multiple streams are used, it is _not_ necessarily the
// case that ascending streams/endpoints in usbdev_stream_test are mapped to
// a contiguous range of ascending ttyUSBi port names.
//
// Either or both of the initial input port and the initial output port may be
// overridden using command line parameters.
//
// Usage:
//   stream [-v<bool>][-c<bool>][-r<bool>][-s<bool>]
//          [[-d<bus>:<address>] | [--device <bus>:<address>]]
//          [<input port>[ <output port>]]
//
//   --device   programmatically specify a particular USB device by bus number
//              and device address (see 'lsusb' output).
//
//   -c   check any retrieved data against expectations
//   -d   specify a particular USB device by bus number and device address
//   -r   retrieve data from device
//   -s   send data to device
//   -t   use serial ports (ttyUSBx) in preference to libusb Bulk Transfer
//        streams for usbdev_stream_test
//   -v   verbose reporting
//   -z   perform suspend-resume signaling throughout the test
//
// <bool> values may be 0,1,n or y, and they default to 1.
//
// Build without libusb dependency:
//   eg. g++ -Wall -Werror -o stream_test *.cc
#include "stream_test.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cinttypes>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sys/time.h>

#include "usb_device.h"
#if STREAMTEST_LIBUSB
#include "usbdev_int.h"
#endif
#include "usbdev_serial.h"
#include "usbdev_utils.h"

// Test properties
//
// 16MiB takes about 40s presently with no appreciable CPU activity on the CW310
// (ie. undefined transmitted data, and no checking of received data) but ca.
// 152s with LFSR generation and checking across all of the 11 streams possible.
//
// Note: in normal use such as regression tests, the stream signatures will
//       override the specified transfer amount.
constexpr uint32_t kTransferBytes = (0x10U << 20);

// Has any data yet been received from the device?
bool received = false;

// Time of first data reception.
uint64_t start_time = 0U;

// Configuration settings for the test.
TestConfig cfg(false,  // Not verbose
               true,   // Retrieve data from the device
               true,   // Check the retrieved data
               true);  // Send modified data to the device

static USBDevice dev;

// State information for each of the streams.
static USBDevStream *streams[STREAMS_MAX];

// Parse a command line option and return boolean value.
static bool GetBool(const char *p);

// Construct a modified port name for the next stream.
static void PortNext(char *next, size_t n, const char *curr);

// Parse a command line option and return boolean value.
bool GetBool(const char *p) {
  return (*p == '1') || (tolower(*p) == 'y') || (*p == '\r') || (*p == '\n') ||
         (*p == '\0');
}

// Parse a command line option, retrieving a byte and indicating
// success/failure.
bool GetByte(const char **pp, uint8_t &byte) {
  const char *p = *pp;
  if (isdigit(*p)) {
    uint32_t n = 0u;
    do {
      n = (n * 10) + *p++ - '0';
    } while (n < 0x100u && isdigit(*p));
    if (n < 0x100u) {
      byte = (uint8_t)n;
      *pp = p;
      return true;
    }
  }
  return false;
}

// Parse a command line option specifying the bus number and device address.
bool GetDevice(const char *p, uint8_t &busNumber, uint8_t &devAddress) {
  return GetByte(&p, busNumber) && (*p++ == ':') && GetByte(&p, devAddress) &&
         *p == '\0';
}

// Construct a modified port name for the next stream.
void PortNext(char *next, size_t n, const char *curr) {
  // We're expecting a port name of the form '/dev/ttyUSB<n>'
  if (curr != next) {
    strncpy(next, curr, n);
  }
  while (*next != '\0') {
    if (isdigit(*next)) {
      int port = atoi(next);
      snprintf(next, n, "%u", (unsigned)port + 1U);
      break;
    }
    next++;
    n--;
  }
}

// Report command line syntax.
void ReportSyntax(void) {
  fputs(
      "Usage:\n"
      "  stream [-n<streams>][-v<bool>][-c<bool>][-r<bool>][-s<bool>][-t][-z]\n"
      "         [[-d<bus>:<address>] | [--device <bus>:<address>]]\n"
      "         [<input port>[ <output port>]]"
      "\n\n"
      "   --device   programmatically specify a particular USB device by bus\n"
      "              number and device address (see 'lsusb' output).\n"
      "\n\n"
      "  -c   check any retrieved data against expectations\n"
      "  -d   specify a particular USB device by bus number"
      " and device address\n"
      "  -r   retrieve data from device\n"
      "  -s   send data to device\n"
      "  -t   use serial ports (ttyUSBx) in preference to libusb Bulk\n"
      "       Transfer streams for usbdev_stream_test\n"
      "  -v   verbose reporting\n"
      "  -z   perform suspend-resume signaling throughout the test"
      "\n\n"
      "  <bool> values may be 0,1,n or y, and they default to 1\n",
      stderr);
}

static int RunTest(USBDevice *dev, const char *in_port, const char *out_port) {
  // We need to modify the port names for each non-initial stream.
  char out_name[FILENAME_MAX];
  char in_name[FILENAME_MAX];

  // Collect the test number and the test arguments so that we may ascertain
  // the transfer type of each of the streams.
  uint8_t testNum = dev->TestNumber();
  uint8_t testArg[4];
  for (unsigned arg = 0U; arg < 4U; arg++) {
    testArg[arg] = dev->TestArg(arg);
  }

  // Determine the number of streams from the test descriptor; the device-side
  // software supplies the stream count.
  unsigned nstreams = 2U;
  switch (testNum) {
    case USBDevice::kUsbTestNumberStreams:
    case USBDevice::kUsbTestNumberIso:
    case USBDevice::kUsbTestNumberMixed:
      // The lower nibble of the first test argument specifies the stream count
      // in these test descriptions.
      nstreams = testArg[0] & 0xfU;
      break;
      // Other tests default to 2 Bulk streams.
    default:
      nstreams = 2U;
      break;
  }

  // Decide upon the number of bytes to be transferred for the entire test.
  uint32_t transfer_bytes = kTransferBytes;
  transfer_bytes = (transfer_bytes + nstreams - 1) / nstreams;
  if (cfg.verbose) {
    std::cout << " - " << nstreams << " stream(s), 0x" << std::hex
              << transfer_bytes << std::dec << " bytes each" << std::endl;
  }

  // Initialize all streams.
  for (unsigned idx = 0U; idx < nstreams; idx++) {
    USBDevStream::StreamType streamType;

    switch (testNum) {
      case USBDevice::kUsbTestNumberStreams:
        // For the basic streaming test where all active endpoints are using
        // Bulk Transfer types, we may either use the ttyUSBn serial port
        // interface or we may use libusb.
        //
        // In the former case we cannot support suspend-resume testing because
        // data will get buffered somewhere within the software layers and
        // lost when the file descriptors are closed and opened.
        if (cfg.serial && !cfg.suspending) {
          streamType = USBDevStream::StreamType_Serial;
        } else {
          streamType = USBDevStream::StreamType_Bulk;
        }
        break;
      case USBDevice::kUsbTestNumberIso:
        streamType = USBDevStream::StreamType_Isochronous;
        break;
      case USBDevice::kUsbTestNumberMixed: {
        uint32_t mixedTypes =
            (testArg[3] << 16) | (testArg[2] << 8) | testArg[1];
        // Two bits per stream specify the stream/transfer type in terms of the
        // USB standard endpoint types.
        switch ((mixedTypes >> (idx * 2)) & 3U) {
          case 0U:
            streamType = USBDevStream::StreamType_Control;
            break;
          case 1U:
            streamType = USBDevStream::StreamType_Isochronous;
            break;
          case 2U:
            streamType = USBDevStream::StreamType_Bulk;
            break;
          default:
            streamType = USBDevStream::StreamType_Interrupt;
            break;
        }
      } break;
      // Other tests default to 2 Bulk streams.
      default:
        streamType = USBDevStream::StreamType_Bulk;
        break;
    }

    std::cout << "S" << idx << ": " << USBDevStream::StreamTypeName(streamType)
              << std::endl;

    bool opened(false);
#if STREAMTEST_LIBUSB
    bool bulk(true);
#endif
    switch (streamType) {
      case USBDevStream::StreamType_Serial: {
        USBDevSerial *s;
        s = new USBDevSerial(idx, transfer_bytes, cfg.retrieve, cfg.check,
                             cfg.send, cfg.verbose);
        if (s) {
          opened = s->Open(in_port, out_port);
          if (opened) {
            streams[idx] = s;

            // Modify the port name for the next stream.
            PortNext(out_name, sizeof(out_name), out_port);
            PortNext(in_name, sizeof(in_name), in_port);
            out_port = out_name;
            in_port = in_name;
          }
        }
      } break;

#if STREAMTEST_LIBUSB
      case USBDevStream::StreamType_Interrupt:
        bulk = false;
        // no break; Bulk Transfers are handled identically to Interrupt
        // Transfers.
      case USBDevStream::StreamType_Bulk: {
        USBDevInt *interrupt;
        interrupt = new USBDevInt(dev, bulk, idx, transfer_bytes, cfg.retrieve,
                                  cfg.check, cfg.send, cfg.verbose);
        if (interrupt) {
          opened = interrupt->Open(idx);
          if (opened) {
            streams[idx] = interrupt;
          }
        }
      } break;
#endif
      default:
        assert(!"Unrecognised/invalid stream type");
        break;
    }

    if (!opened) {
      std::cerr << "Failed to open stream" << std::endl;
      if (idx > 0U) {
        do {
          idx--;
          delete streams[idx];
        } while (idx > 0U);
      }
      return 1;
    }
  }

  std::cout << "Streaming...\r" << std::flush;

  // Times are in microseconds.
  constexpr uint32_t kRunInterval = 5 * 1000000;  // Running before suspending.
  constexpr uint32_t kSuspendingInterval = 5 * 1000;    // Suspending.
  constexpr uint32_t kSuspendedInterval = 5 * 1000000;  // Device is suspended.
  // Resume Signaling shall occur for at least 20ms but we have no control.
  // over its duration, so there's little point trying to communicate sooner.
  constexpr uint32_t kResumeInterval = 30 * 1000;  // Resuming before traffic.
  uint64_t start_time = time_us();
  uint32_t prev_bytes = 0;
  bool done = false;
  do {
    uint32_t total_bytes = 0U;
    uint32_t total_recv = 0U;
    uint32_t total_sent = 0U;
    bool failed = false;

    done = false;
    switch (dev->CurrentState()) {
      case USBDevice::StateStreaming:
        done = true;
        for (unsigned idx = 0U; idx < nstreams; idx++) {
          // Service this stream.
          if (!streams[idx]->Service()) {
            failed = true;
            break;
          }

          // Update the running totals.
          total_bytes += streams[idx]->TransferBytes();
          total_recv += streams[idx]->BytesRecvd();
          total_sent += streams[idx]->BytesSent();

          // Has the stream completed all its work yet?
          if (!streams[idx]->Completed()) {
            done = false;
          }
        }

        // Initiate transition to Suspended.
        if (cfg.suspending && elapsed_time(start_time) >= kRunInterval) {
          std::cout << "Waiting to suspend" << std::endl;
          // Notify all of the streams that no more traffic shall be initiated.
          for (unsigned idx = 0U; idx < nstreams; idx++) {
            streams[idx]->Pause();
          }
          if (true) {
            // Initiate autosuspend.
            dev->Suspend();
            // Start of Suspending interval.
            start_time = time_us();
          } else {
            // TODO: There remains an issue in which the host-side software
            // apparently fails to complete one or more transfers, which
            // prevents us from properly resuming the transfers after the device
            // has cycled through suspend-resume; this code persists in order
            // to demonstrate and further investigate that.
            //
            // At the time the issue appeared to be in the behavior of the
            // driver stack/libusb.

            std::cout << "Attempting to resume" << std::endl;
            // Notify all of the streams that no more traffic shall be
            // initiated.
            for (unsigned idx = 0U; idx < nstreams; idx++) {
              streams[idx]->Resume();
            }
            std::cout << "Resuming streaming..." << std::endl;
            // Start of Running interval.
            start_time = time_us();
          }
        }
        break;

      // Put the device into Suspended for a while.
      case USBDevice::StateSuspending:
        if (elapsed_time(start_time) >= kSuspendingInterval) {
          dev->SetState(USBDevice::StateSuspended);
          // Start of Suspended interval.
          start_time = time_us();
          std::cout << "Suspended" << std::endl;
        }
        break;

      case USBDevice::StateSuspended:
        if (elapsed_time(start_time) >= kSuspendedInterval) {
          dev->Resume();
          // Start of Resuming interval.
          start_time = time_us();
        }
        break;

      case USBDevice::StateResuming:
        if (elapsed_time(start_time) >= kResumeInterval) {
          for (unsigned idx = 0U; idx < nstreams; idx++) {
            streams[idx]->Resume();
          }

          dev->SetState(USBDevice::StateStreaming);
          // Start of Running interval.
          start_time = time_us();
        }
        break;
    }

    // Service the USBDevice to keep USB transfers flowing.
    if (!failed) {
      failed = !dev->Service();
    }

    // Tidy up if something went wrong.
    if (failed) {
      for (unsigned idx = 0U; idx < nstreams; idx++) {
        (void)streams[idx]->Stop();
      }
      return 3;
    }

    // Down counting of the number of bytes remaining to be transferred.
    if (std::abs((int32_t)total_sent - (int32_t)prev_bytes) >= 0x1000 || done) {
      // Note: if there are Isochronous streams present then the bytes left
      // count may hit zero some time before the test completes on the device
      // side because packet delivery is not guaranteed.
      uint32_t bytes_left =
          (total_sent < total_bytes) ? (total_bytes - total_sent) : 0U;
      std::cout << "Bytes received: 0x" << std::hex << total_recv
                << " -- Left to send: 0x" << bytes_left << "         \r"
                << std::dec << std::flush;
      prev_bytes = total_sent;
    }
  } while (!done);

  uint64_t elapsed_time = time_us() - start_time;

  // Report time elapsed from the start of data transfer.
  for (unsigned idx = 0U; idx < nstreams; idx++) {
    streams[idx]->Stop();
  }

  // TODO: introduce a crude estimate of the performance being achieved,
  // for profiling the performance of IN and OUT traffic; totals and individual
  // endpoints?
  double elapsed_secs = elapsed_time / 1e6;
  printf("Test completed in %.2lf seconds (%" PRIu64 "us)\n", elapsed_secs,
         elapsed_time);

  return 0;
}

int main(int argc, char *argv[]) {
  const uint16_t kVendorID = 0x18d1u;
  const uint16_t kProductID = 0x503au;
  const char *out_port = nullptr;
  const char *in_port = nullptr;
  uint8_t devAddress = 0u;
  uint8_t busNumber = 0u;

  cfg.override_flags = false;

  // Collect options and alternative port names.
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (tolower(argv[i][1])) {
        case 'c':
          cfg.check = GetBool(&argv[i][2]);
          cfg.override_flags = true;
          break;
        case 'd':
          if (!GetDevice(&argv[i][2], busNumber, devAddress)) {
            std::cerr << "ERROR: Unrecognised option '" << argv[i] << "'"
                      << std::endl;
            ReportSyntax();
            return 7;
          }
          break;
        case 'r':
          cfg.retrieve = GetBool(&argv[i][2]);
          cfg.override_flags = true;
          break;
        case 's':
          cfg.send = GetBool(&argv[i][2]);
          cfg.override_flags = true;
          break;
        case 't':
          cfg.serial = GetBool(&argv[i][2]);
          break;
        case 'v':
          cfg.verbose = GetBool(&argv[i][2]);
          break;
        case 'z':
          cfg.suspending = GetBool(&argv[i][2]);
          break;
        case '-':
          // The bus/address may be specified programmatically as '--device'
          // with confidence that this parameter/syntax will not change.
          if (!strcmp(&argv[i][2], "device") && i < argc - 1) {
            // The next argument should be 'bus:address'
            if (GetDevice(argv[++i], busNumber, devAddress)) {
              break;
            }
          }
          // no break
        default:
          std::cerr << "ERROR: Unrecognised option '" << argv[i] << "'"
                    << std::endl;
          ReportSyntax();
          return 6;
      }
    } else if (!out_port) {
      out_port = argv[i];
    } else if (!in_port) {
      in_port = argv[i];
    } else {
      std::cerr << "ERROR: Parameter '" << argv[i] << "' unrecognised"
                << std::endl;
      ReportSyntax();
      return 7;
    }
  }

  // Furnish test with default port names.
  if (!out_port) {
    out_port = "/dev/ttyUSB0";
  }
  if (!in_port) {
    in_port = "/dev/ttyUSB0";
  }

  std::cout << "USB Streaming Test" << std::endl
            << " (host-side implementation of usbdev streaming tests)"
            << std::endl;

  // Locate the USB device using Vendor and Product IDs, and optionally a
  // specific device address and bus number to handle the presence of multiple
  // similar devices.
  USBDevice dev(cfg.verbose);
  if (!dev.Init(kVendorID, kProductID, devAddress, busNumber)) {
    return 2;
  }

  if (!dev.Open()) {
    dev.Fin();
    return 3;
  }

  // Read a vendor-specific test descriptor from the device-side software in
  // order to ascertain the test configuration and required behavior.
  if (!dev.ReadTestDesc()) {
    dev.Fin();
    return 3;
  }

  int rc = RunTest(&dev, in_port, out_port);

  dev.Fin();

  return rc;
}
