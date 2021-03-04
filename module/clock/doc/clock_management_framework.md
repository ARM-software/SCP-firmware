\ingroup GroupModules Modules
\defgroup GroupClock Clock HAL


# Clock Management Framework


## Overview

As platform requirements get more complex the SCP must be able to support the
associated clock topologies. This implementation allows to declare the
available clocks and their association to each other in the Clock Tree
(preferred).
Whenever a state change request is received the clock framework will enable and
disable the relevant clock nodes to fulfil the request.

## Considerations

This simple set of rules encapsulates the clock management philosophy.

### Rules for setting state

- If one clock in the tree is enabled then all parent clocks in the tree are
enabled.
- If one clock in the tree is disabled, then all children clocks in the tree must be disabled.
- A clock must not be enabled until all the assigned parent clocks in the
clock tree have been enabled.
- A clock must not be disabled if any of its children clocks are enabled.

---

## Setting state operation (enable/disable)

A clock operation may be initiated either by:
- SCMI request: A standard request defined by the SCMI Clock Protocol.
- Event: A clocks event handler for any operation may initiate a clock
operation on the current clock.

A clock operation will respond to each request with either of:
- SCMI response: A standard response defined by the SCMI Clock Protocol.
- Event: Every operation event received by the current clock will be ACKed
with a status.

### Operation example
In the following example we consider enable and disable functionality.
#### Clock tree
```
+-----------+
|   CLOCK-0 |
+-+---------+
  |
  |  +------------+
  +-->    CLOCK-1 |
     +-+----------+
       |
       |
       |   +------------+
       +--->    CLOCK-2 |
       |   +------------+
       |
       |   +------------+
       +--->    CLOCK-3 |
           +------------+
```

#### Enable __Clock-2__

In this scenario, it is assumed that all clocks before the operation are
__disabled__.

1. OSPM Agent sends SCMI request to enable Clock-2.
2. Clock-2 sends Pre-Enable Event to Clock-1.
3. Clock-1 sends Pre-Enable Event to Clock-0.
4. Clock-0 enable operation completes and increase internal reference count.
5. Clock-0 sends Post-Enable Event ACK to Clock-1.
6. Clock-1 enable operation completes and increase internal reference count.
7. Clock-1 sends Post-Enable Event ACK to Clock-2.
8. Clock-2 enable operation completes and increase internal reference count.
9. Clock-2 responds to original SCMI request.

NOTE: In case the enable fails, the active clock node sends a post event to the
parent, so the parent can decrease the reference count, which was incremented
earlier.

```
  OSPM      CLOCK-0    CLOCK-1    CLOCK-2    CLOCK-3
  AGENT
    +          +          +          +          +
    |          |          |          |          |
1)  +------------------------------->+          |
    |          |          |          |          |
2)  |          |          +<---------+          |
    |          |          |          |          |
3)  |          +<---------+          |          |
    |          |          |          |          |
4)  |          +---+      |          |          |
    |          |   |      |          |          |
    |          +<--+      |          |          |
    |          |          |          |          |
5)  |          +--------->+          |          |
    |          |          |          |          |
6)  |          |          +---+      |          |
    |          |          |   |      |          |
    |          |          +<--+      |          |
    |          |          |          |          |
7)  |          |          +--------->+          |
    |          |          |          |          |
8)  |          |          |          +---+      |
    |          |          |          |   |      |
    |          |          |          +<--+      |
    |          |          |          |          |
9)  +<-------------------------------+          |
    |          |          |          |          |
    +          +          +          +          +
```

#### Disable __Clock-1__ - Scenario 1
In this scenario, it is assumed that all clocks before the operation
are __enabled__. Clock 1 was externally set and it has its reference count
equals to 3.

1. OSPM Agent sends SCMI request to disable Clock-1
2. Clock-1 checks if reference count is 0.
    - Hence it does not turn off Clock-1.
    - Clock-1 decreases its internal reference count.
3. Clock-1 responds to original SCMI request.


```
  OSPM      CLOCK-0    CLOCK-1    CLOCK-2    CLOCK-3
  AGENT
    +          +          +          +          +
    |          |          |          |          |
1)  +-------------------->+          |          |
    |          |          |          |          |
2)  |          |          +---+      |          |
    |          |          |   |      |          |
    |          |          +<--+      |          |
    |          |          |          |          |
3)  +<--------------------+          |          |
    |          |          |          |          |
    +          +          +          +          +

```

#### Disable __Clock-2__ - Scenario 2
In this scenario is assumed that clocks 0, 1 and 2 are __enabled__ before the
operation and clock 3 is disabled. Clock 2 was externally set and
it has its reference count equals to 1.


1. OSPM Agent sends SCMI request to disable Clock-2
2. Clock-2 decreases internal reference count and checks if reference count
is 0.
    - Clock-2 disable operation completes.
3. Clock-2 responds to original SCMI request.
4. Clock-2 sends Post-Disable Event to Clock-1.
5. Clock-1 decrements its reference count. It has reference count equal to 0.
    - Clock-1 disable operation completes.
6. Clock-1 sends Post-Disable Event to Clock-0.
7. Clock-0 decrements its reference count. It has reference count equal to 0.
    - Clock-0 disable operation completes.

```
  OSPM      CLOCK-0    CLOCK-1    CLOCK-2    CLOCK-3
  AGENT
    +          +          +          +          +
    |          |          |          |          |
1)  +------------------------------->+          |
    |          |          |          |          |
2)  |          |          |          +---+      |
    |          |          |          |   |      |
    |          |          |          +<--+      |
    |          |          |          |          |
3)  +<-------------------------------+          |
    |          |          |          |          |
4)  |          |          +<---------+          |
    |          |          |          |          |
5)  |          |          +---+      |          |
    |          |          |   |      |          |
    |          |          +<--+      |          |
    |          |          |          |          |
6)  |          +<---------+          |          |
    |          |          |          |          |
7)  |          +---+      |          |          |
    |          |   |      |          |          |
    |          +<--+      |          |          |
    |          |          |          |          |
    +          +          +          +          +

```

## Setting rate operation

A clock set rate operation may be initiated by a SCMI request. A clock operation
will respond to each request with a SCMI response.
During this operation the selected clock node will notify every children clock
with its new output rate (which is the new input rate for its children). Later,
every child clock can decide how to behave accordingly.

### Operation example
#### Clock arrangement

```
+-----------+
|   CLOCK-0 |
+-+---------+
  |
  |  +------------+
  +-->    CLOCK-1 |
     +-+----------+
       |
       |
       |   +------------+
       +--->    CLOCK-2 |
           +------------+
```
#### Initial rates

* Clock-0 @ 300MHz
* Clock-1 = A/3
* Clock-2 = B/2

```
+-----------+
|   CLOCK-0 | @ 300MHz
+-+---------+
  |
  |  +------------+
  +-->    CLOCK-1 | @ 100MHz
     +-+----------+
       |
       |
       |   +------------+
       +--->    CLOCK-2 | @ 50MHz
           +------------+
```
1. OSPM Agent sends SCMI request to set Clock-0 @ 180MHz.
2. Clock-0 responds to original SCMI request.
3. Clock-0 sends an Event to Clock-1 with its new rate.
4. Clock-1 updates its output rate. Clock-1 @ 60MHz.
5. Clock-1 sends an Event to Clock-2 with its new rate.
6. Clock-2 updates its output rate. Clock-2 @ 30MHz.

## Limitations

At this time, the clock tree management only supports synchronous drivers. Using
asynchronous drivers terminate execution. It is not recommended to use at this
time.

## Appendix

### Configuration example

To configure a clock tree as it is described above. The build flag
```BS_FIRMWARE_HAS_CLOCK_TREE_MGMT``` should be set and ```parent_id```
should be defined.

```config_mod_clock.c```

```C
static const struct fwk_element clock_dev_desc_table[] = {
...
    [CLOCK_IDX_MOCK_0] =
        {
            .name = "MOCK_CLOCK_0",
            .data = &((struct mod_clock_dev_config){
                ...
            }),
        },
    [CLOCK_IDX_MOCK_1] =
        {
            .name = "MOCK_CLOCK_1",
            .data = &((struct mod_clock_dev_config){
                ...
                .parent_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_MOCK_0),
            }),
        },
    [CLOCK_IDX_MOCK_2] =
        {
            .name = "MOCK_CLOCK_2",
            .data = &((struct mod_clock_dev_config){
                ...
                .parent_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_MOCK_1),
            }),
        },
    [CLOCK_IDX_MOCK_3] =
        {
            .name = "MOCK_CLOCK_3",
            .data = &((struct mod_clock_dev_config){
                ...
                .parent_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_MOCK_1),
            }),
        },
...
};
```

```firmware.mk```

```
...
BS_FIRMWARE_HAS_CLOCK_TREE_MGMT := yes
...
```

NOTE: There are no limitations on the clock exposed via SCMI (with respect to
their position on the tree). Every clock node can be exposed to agents to
controlled externally.