Copyright (c) 2022, Arm Limited. All rights reserved.

# N1SDP Platform

## Build configuration options {#build-configuration}
Platform specific option has been added for N1SDP SoC platform.

- SCP_N1SDP_SENSOR_LIB_PATH: Path of the n1sdp sensor library

Example build command -

> ```sh
>  make -f Makefile.cmake PRODUCT=n1sdp \
>  EXTRA_CONFIG_ARGS="-DSCP_N1SDP_SENSOR_LIB_PATH=<Path to sensor library>"
> ```

If SCP_N1SDP_SENSOR_LIB_PATH is set to the path of the sensor
library, then functions defined in the library are used else the
functions defined in the `n1sdp_sensor_driver.c` file are used.
