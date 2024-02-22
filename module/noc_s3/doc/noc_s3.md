\ingroup GroupModules Modules
\defgroup GroupNoCS3 NoCS3

# Module NoCS3 Architecture

Copyright (c) 2024, Arm Limited. All rights reserved.

# Overview

NoC S3 Network-on-Chip Interconnect is a highly configurable system-level
interconnect. This design allows the creation of a highly optimized,
non-coherent interconnect which supports Programmable System Address Map,
Programmable Access Protection Unit, Programmable Fault Protection Unit and
many more.

```ditaa {cmd=true args=["-E"]}
                  +---------------------------------------+
                  |                NoC S3                 |
                  |                                       |
                  |                                       |
                  |                                       |
                  |   Completer Node      Requester Node  |
                  |   for Requester       For completer   |
 +------------+   |   +----------+        +-----------+   |    +-----------+
 |            |   |   |          |        |           |   |    |           |
 |  Requester +-->+-->|xSNI Node +-- -- --+ xMNI Node +-->+--->| Completer |
 |            |   |   |          |        |           |   |    |           |
 +------------+   |   | PSAM/APU |        |   APU     |   |    +-----------+
                  |   +----------+        +-----------+   |
                  |                                       |
                  |                                       |
                  |                                       |
                  |                                       |
                  |                                       |
                  +---------------------------------------+
```

This module implements the driver for NoC S3 runtime configuration.

# Module Design

This module provides an interface for other modules to configure a given NoC S3
instance during runtime. The module does that in 2 stages.
1) Node Discovery
2) PSAM programming

# Node Discovery

The configuration register layout for NoC S3 is laid out in a multibranch tree
format. On the top of the tree is global configuration node which points to the
domain and domains points to components nodes.  The components nodes contains
subfeatures that can be used to enable PSAM, APU, FCU and IDM.

The discovery process is designed to traverse a tree and convert the node data
into a table for easy and optimized access in O(1) time. The table is used to
store the offset of each node. During the discovery process, the tree is parsed
twice. The first time it is parsed, the maximum row size for all node types is
found, and during the second pass, the offset for each discovered node type is
recorded. The node IDs within the same types are linear, and the maximum node
ID can be used to determine the total number of nodes.

The discovery process will convert the following tree
```ditaa {cmd=true args=["-E"]}
                        Global CFGNI
                        /         \
                       /           \
                     VD0           VD1
                    /              /
                  PD0             PD1
                 /               /
               CD0              CD1
              /  \             /
             C0  C1           C2

```
To the following table containing node offset.
```ditaa {cmd=true args=["-E"]}
 _______________________________________________
|Node Type      | Index  |      Node Id         |
|---------------|--------|----------------------|
|               |        |   0   |   1   |  2   |
|---------------|--------|-------|-------|------|
|Global CFGNI   |   0    |CFGNI0 |       |      |
|Voltage Domain |   1    |VD0    |VD1    |      |
|Power Domain   |   2    |PD0    |PD1    |      |
|CLock Domain   |   3    |CD0    |CD1    |      |
|ASNI           |   4    |C0     |C1     |C2    |
|AMNI           |   5    |       |       |      |
|PMU            |   6    |       |       |      |
|HSNI           |   7    |       |       |      |
|HMNI           |   8    |       |       |      |
|PMNI           |   9    |       |       |      |
|_______________|________|_______|_______|______|
```

As the node id and type are predefined, the offset of the target node is
fetch in O(1) by using id and type.
Example: To get offset of the C2, the user will access the table using node
type 4 and Id 2. Table[4][2] will give offset of C2.

