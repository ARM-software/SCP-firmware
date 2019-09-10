\ingroup GroupModules Modules
\defgroup GroupSCMI_APCORE SCMI Core Configuration Protocol

SCMI AP Core Configuration Protocol v1.0
========================================

Protocol Overview                               {#scmi_apcore_protocol_overview}
=================

This protocol is an extension of the [Arm System Control and Management
Interface (SCMI)]
(http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/index.html).

The goal of this protocol is for the SCP to provide an interface to the AP
firmware that supports changing the configuration of one or more AP cores.
For example, one of the supported commands allows programming of the AP core
address and is used when AP firmware is unable to program the reset address
directly.

The protocol identifier used for this protocol (0x90) is within the range that
the SCMI specification provides for platform-specific extensions (0x80 - 0xFF).
For further information on protocol identifiers refer to section 4.1.2 of the
SCMI specification.

Protocol Commands                                        {#scmi_apcore_protocol}
=================

Protocol Version                                 {#scmi_apcore_protocol_version}
----------------

On success, this command returns the version of the protocol. For this version
of the specification the return value must be 0x10000, which corresponds to 1.0.

message_id: 0x0<br>
protocol_id: 0x90

This command is mandatory.

Return values:
* int32 status
    * See section 4.1.4 of the SCMI specification for status code
      definitions
* uint32 version
    * For this version of the specification the return value must be 0x10000

Protocol Attributes                           {#scmi_apcore_protocol_attributes}
-------------------

This command returns the implementation details associated with this protocol.

message_id: 0x1<br>
protocol_id: 0x90

This command is mandatory.

Return values:
* int32 status
    * See section 4.1.4 of the SCMI specification for status code
      definitions
* uint32 attributes
    * Bits [31:1] Reserved, must be zero.
    * Bit [0] If set to 1, the platform supports 64-bit reset addresses. If set
      to 0, the platform supports 32-bit reset addresses.

Protocol Message Attributes          {#scmi_apcore_protocol_message__attributes}
---------------------------

On success, this command returns the implementation details associated with a
specific message in this protocol. In addition to the standard status codes
described in section 4.1.4 of the SCMI specification, the command can return the
error NOT_FOUND if the message identified by message_id is not provided by
the implementation.

message_id: 0x2<br>
protocol_id: 0x90

This command is mandatory.

Parameters:
* uint32 message_id
    * message_id of the message.

Return values:
* int32 status
    * See section 4.1.4 of the SCMI specification for status code
      definitions.
* uint32 attributes
    * Flags associated with a specific command in the protocol. For all commands
      in this protocol this parameter has a value of 0.

Core Reset Address Set                       {#scmi_apcore_protocol_set_address}
----------------------

Set the application core reset address. The address applies to all cores.

In some platforms only the SCP is capable of programming the application core
reset address. This command allows the SCP to carry out the programming on
behalf of AP firmware. Such a feature is supported in the [Arm Trusted Firmware]
(https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/reset-design.rst)

message_id: 0x3<br>
protocol_id: 0x90

This command is optional.

Parameters:
* uint32 Reset address (lower word)
    * Bit[31:0] AP core reset address (low)
* uint32 Reset address (higher word)
    * Bit[31:0] AP core reset address (high)
    * On platforms that support only 32-bit addresses, only the lower word is
      used - this higher word must be zero and the address must be 4-byte
      aligned. For platforms supporting 64-bit addresses both words may be used
      and the address must be 8-byte aligned.
* uint32 attributes
    * Bit[31:1] Reserved, must be zero.
    * Bit[0] Lock. When set to 1, the platform will deny any further attempts to
      change the reset address.

Return values:
* int32 status
    * SUCCESS if the reset address was set successfully.
    * INVALID_PARAMETERS:
        * Reset address alignment is invalid.
        * Platform supports only 32-bit addresses and the reset address received
          is larger than 32-bits.
    * DENIED: The reset address is locked and changes are not permitted.
    * DENIED: The calling agent is not permitted to modify the reset address.
    * See section 4.1.4 of the SCMI specification for status code
      definitions.

Core Reset Address Get                       {#scmi_apcore_protocol_get_address}
----------------------

Get the application core reset address. The address applies to all cores.

message_id: 0x4<br>
protocol_id: 0x90

This command is optional.

Return values:
* int32 status
    * SUCCESS if the reset address was retrieved successfully.
    * DENIED: The calling agent is not permitted to retrieve the reset address.
    * See section 4.1.4 of the SCMI specification for status code
      definitions.
* uint32 Reset address (lower word)
    * Bit[31:0] AP core reset address (low)
* uint32 Reset address (higher word)
    * Bit[31:0] AP core reset address (high)
    * On platforms that support only 32-bit addresses, only the lower word is
      used and this higher word must be zero.
* uint32 attributes
    * Bit[31:1] Reserved, must be zero.
    * Bit[0] Lock. When set to 1, changing the reset address is not permitted.
