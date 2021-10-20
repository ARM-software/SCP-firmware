Device tree python scripts and library
--------------------------------------

The dt_gen_config.py is a script based largely on the on the equivalent
script in the Zephyr project (zephyr/scripts/dts/gen_defines.py).

This generated C #defines from devicetree files to provide a complile time
extraction of device tree information.

The main device tree library is located in scp/contrib/python-devicetree
This is copied as is from zephyr/scripts/dts/python-devicetree.

The README.txt in zephyr/scripts/dts indicates that there is a plan to make
the library standalone an installable from PyPi

Once this occurs then it can be installed on a build machine / copied from there
rather than be part of the SCP project repository.



