# Morello Platform

## Build configuration options {#build-configuration}
Platform specific option has been added for Morello SoC platform.

- SCP_MORELLO_SENSOR_LIB_PATH: Path of the morello sensor library

If SCP_MORELLO_SENSOR_LIB_PATH is set to the path of the sensor
library, then functions defined in the library are used else the
functions defined in the `morello_sensor_driver.c` file are used.
