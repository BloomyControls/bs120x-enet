# BS120x Ethernet Driver

C/C++ driver for communicating with a Bloomy Controls BS120x-series battery
cell simulators over Ethernet.

> [!IMPORTANT]
> Currently, this library only supports BS1201. If you are using BS1200, contact
> Bloomy for firmware updates to support this driver.

## Features

- Modern, native C++20 library
- Supports Windows and Linux
- Implements all commands and queries supported by the BS1201
- Exception-less error handling (see below)
- C wrapper (`include/bci/bs120x/CInterface.h`) for use in C and other languages
- Easy inclusion in CMake projects

> [!NOTE]
> Exceptions are not handled internally unless it is necessary to do so. Any
> exceptions thrown by this library are considered critical exceptional
> situations, such as a failure to allocate memory.

## Example Usage (C++)

This example establishes a connection to the device and then queries and reports
its current configuration information. This library makes use of Sy Brand's
excellent `expected` library to provide exception-less error handling. If this
library throws an exception, it is considered an exceptional event, and is
generally expected to be unrecoverable.

```cpp
#include <bci/bs120x/EthernetClient.h>

#include <chrono>
#include <iostream>
#include <thread>

int main() {

  using ec = bci::bs120x::ErrorCode;
  bci::bs120x::EthernetClient driver;

  auto ret = driver.Connect("192.168.1.105",  // interface IP
                            "192.168.1.101",  // device IP
                            54321,            // UDP port
                            1000,             // UDP timeout (ms)
                            12345);           // TCP port
  if (ret != ec::kSuccess){
    std::cerr << "Failed to connect to unit: "
        << bci::bs120x::ErrorMessage(ret) << std::endl;
    return 1;
  }

  ret = driver.QueryConfig();
  if (ret != ec::kSuccess) {
    std::cerr << "Failed to query configuration: "
        << bci::bs120x::ErrorMessage(ret) << std::endl;
    return 1;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  if (auto cfg = driver.GetUnitConfig()) {
    std::cout << std::boolalpha;
    std::cout << "Serial number: " << cfg->serial_number << '\n';
    std::cout << "Firmware version: " << cfg->firmware_version << '\n';
    std::cout << "Calibration date: " << cfg->calibration_date << '\n';
    std::cout << "Cell inhibit enable: " << cfg->cell_inhibit_enable << '\n';
    std::cout << "IP address: " << cfg->ip_address << '\n';
    std::cout << "UDP data port: " << cfg->udp_data_port << '\n';
    std::cout << "UDP data period: " << cfg->udp_data_period << '\n';
    std::cout << "UDP data broadcast enable: " << cfg->udp_data_broadcast_enable
        << '\n';
    std::cout << "Box ID: " << cfg->box_id << '\n';
    std::cout << "CAN data period: " << cfg->can_data_period << '\n';
  } else {
    std::cerr << "Failed to get unit configuration: "
        << bci::bs120x::ErrorMessage(cfg.error()) << std::endl;
    return 1;
  }
}
```

## Example Usage (C)

This example implements the same logic as the C++ example above using the
C interface.

```c
#include <bci/bs120x/CInterface.h>

#include <inttypes.h>
#include <stdio.h>
#include <threads.h>
#include <time.h>

static void FormatErr(const char* msg, int ec) {
  fprintf(stderr, "%s: %s (%d)\n", msg, Bs120xEnet_ErrorMessage(ec), ec);
}

int main(void){
  int ret = 1;
  int ec;
  Bs120xEnetHandle handle = NULL;
  struct Bs120xUnitConfig cfg;

  ec = Bs120xEnet_Init(&handle);
  if (ec < 0) {
    FormatErr("Failed to initialize client", ec);
    goto cleanup;
  }

  ec = Bs120xEnet_Connect(handle,           // handle
                          "192.168.1.105",  // interface IP
                          "192.168.1.101",  // device IP
                          54321,            // UDP port
                          1000,             // UDP timeout (ms)
                          12345);           // TCP port
  if (ec < 0) {
    FormatErr("Failed to connect to unit", ec);
    goto cleanup;
  }

  ec = Bs120xEnet_QueryConfig(handle);
  if (ec < 0) {
    FormatErr("Failed to query configuration", ec);
    goto cleanup;
  }

  // sleep for 100ms using C11 stdlib utilities
  thrd_sleep(&(struct timespec){.tv_nsec = 100000000}, NULL);

  ec = Bs120xEnet_GetUnitConfig(handle, &cfg);
  if (ec < 0) {
    FormatErr("Failed to get unit configuration", ec);
    goto cleanup;
  }

  printf("Serial number: %s\n", cfg.serial_number);
  printf("Firmware version: %s\n", cfg.firmware_version);
  printf("Calibration date: %s\n", cfg.calibration_date);
  printf("Cell inhibit enable: %d\n", cfg.cell_inhibit_enable);
  printf("IP address: %s\n", cfg.ip_address);
  printf("UDP data port: " PRIu16 "\n", cfg.udp_data_port);
  printf("UDP data period: " PRIu32 "\n", cfg.udp_data_period);
  printf("UDP data broadcast enabled: %d\n", cfg.udp_data_broadcast_enable);
  printf("Box ID: " PRIu8 "\n", cfg.box_id);
  printf("CAN data period: " PRIu32 "\n", cfg.can_data_period);

  Bs120xEnet_Disconnect(handle);

  ret = 0;

cleanup:
  Bs120xEnet_Destroy(&handle);
  return ret;
}
```

## Building

To build the driver as a shared library (.DLL or .so), first configure CMake,
then execute the build system. This project can also be built with Visual
Studio.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
cmake --build build
```

## Adding to a CMake Project

In your CMakeLists.txt, fetch and link with the driver:

```cmake
FetchContent_Declare(bs120xenet
  GIT_REPOSITORY https://github.com/BloomyControls/bs120x-enet.git
  GIT_TAG v1.1.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(bs120xenet)

target_link_libraries(my_program PRIVATE bci::bs120xenet)
```

If you have already installed the driver to a standard location on your system,
you can use `find_package` instead:

```cmake
find_package(bs120xenet 1.1.0 REQUIRED)

target_link_libraries(my_program PRIVATE bci::bs120xenet)
```

> [!TIP]
> On Windows, the MSI installer will register the CMake package in the Windows
> registry, so the above example will build and link successfully. However,
> because the DLL is not located in the `PATH` environment variable, the DLL
> will fail to be found at runtime. To fix this, you can add a CMake command to
> copy the DLL to the build directory and you can add an install target to copy
> the DLL when installing your target.

## Dependencies

This library does not have any runtime dependencies aside from the standard
C/C++ runtime libraries on your system. However, it does have some build
dependencies. If these are installed on your system, CMake will find and use
them if your installed versions are compatible. Otherwise, it will automatically
fetch and build them as part of the build. The build dependencies are:

- fmt v12.1.0
- Boost v1.88.0

These are linked statically, regardless of the build configuration.

## License

Copyright (c) 2026, Bloomy Controls, Inc. All rights reserved.

This software is distributed under the BSD-3-clause license. See the LICENSE
file or <https://opensource.org/license/BSD-3-Clause> for details.
