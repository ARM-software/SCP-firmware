
# AMU Interface
Copyright (c) 2023, Arm Limited. All rights reserved.
## Overview
The `AMU` interface provides an API to be implemented by multiple modules
(`AMU` drivers).


This will allow multiple modules to have the same interface while the
implementation is different.
i.e. It abstracts the knowledge of the API user from the implementation,
which makes it more flexible and platform-agnostic.

The `AMU` API retrieves the AMU data from the hardware.
The implementation will copy any number of the available counters into the
buffer provided by the caller. It's the caller's responsibility to ensure
the buffer (memory) is valid and not overridden during the API call.
## Use
To use the AMU interface it requires to add the include path in the
respective module `CMakeLists.txt` file.
``` CMAKE
target_include_directories(${SCP_MODULE_TARGET} PUBLIC
                          "${CMAKE_SOURCE_DIR}/interface/amu/")
```
Then simply include `interface_amu.h` file to use all AMU interface
definitions.

### Example
```C
    /* `api` holds the concrete implementation of the interface. */
   api->get_counters(start_counter_id, amu_buff, num_counters);
```
