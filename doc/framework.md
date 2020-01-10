# Framework Guide

Copyright (c) 2011-2020, Arm Limited. All rights reserved.

This guide covers the framework that is used to implement the SCP/MCP Software
and which can also be used to extend the provided implementation. Each of the
key components that are used by the framework will be defined and explained and,
following this, the way that these components interact within the constraints of
the framework will be explored.

The key components are explained at a relatively high level; for more detailed
descriptions of the framework components, refer to the source-level Doxygen
documentation. If this has not yet been built then the readme.md file provides
information on getting started from scratch. This file can be found in the
root of the SCP/MCP Software directory.

Arm welcomes any feedback on the SCP/MCP Software, including on this
documentation. To provide feedback or to request support please contact Arm by
email at support@arm.com. Arm licensees may also contact Arm via their partner
managers.

## Framework Components

The framework defines several component types that are combined to produce a
complete product. These components have different roles within the product and
different limitations and guidelines that govern their use. This guide explains
the concept of a product itself and then examines each of the components that
can be used when constructing a product using the framework.

### Products

A product is a representation of a system that one or more firmware images
are being produced and built for. For example, a system that implements
both a System Control Processor (SCP) and a Manageability Control Processor
(MCP) may require a separate firmware image for each processor. Alternatively,
a product may be designed such that software for a single processor is split
into multiple images.

A product always consists of a *product.mk* file that defines one or more
firmware targets. Each of these firmware targets is built, producing a binary
image, when building the product as a whole. Refer to the Build System document
for further information on the makefiles that are used by the framework.

In addition to these targets a product may also contain header files that are
used for product-specific definitions.

The directory structure of a project can be seen below, note that there is one
directory per product under the product directory.

\code
root/
    product/
        productname/
            include/
            src/
            module/
            firmware_a/
            firmware_b/
            product.mk
\endcode

### Firmware

Within a product there will always be at least one firmware. A firmware
represents a software image that is built as part of a product. Each firmware
lists the modules that will be built into its image and provides configuration
data for each of these modules.

For each firmware, linker information must be provided in a *fmw_memory.h* file:

- FIRMWARE_MEM_MODE: The desired memory region configuration. Can be one of the
    following:
    - FWK_MEM_MODE_SINGLE_REGION
    - FWK_MEM_MODE_DUAL_REGION_RELOCATION
    - FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
- FIRMWARE_MEM0_BASE: The base address of the MEM0 region, which is always
    used regardless of the memory region configuration given by
    *FIRMWARE_MEM_MODE*.
- FIRMWARE_MEM0_SIZE: The size of the MEM0 region in bytes.
- FIRMWARE_STACK_SIZE (if multithreading enabled): The size of each individual
    thread stack in bytes.

If a dual-region memory configuration is used then *FIRMWARE_MEM1_BASE* and
*FIRMWARE_MEM1_SIZE* must also be defined.

It is the responsibility of the firmware to define - in its Makefile - the
architecture target for the image (using *BS_FIRMWARE_CPU*) and whether the
firmware is multithreading-enabled (using *BS_FIRMWARE_HAS_MULTITHREADING*)
and/or has notification support (using *BS_FIRMWARE_HAS_NOTIFICATION*). The
firmware can optionally define the count of notification subscriptions
(using *BS_FIRMWARE_NOTIFICATION_COUNT*) to be supported.

An example of a simple firmware directory which contains configuration files for
two modules, the firmware.mk file, and the linker script.

\code
firmware_root/
    config_module_a.c
    config_module_b.c
    firmware.mk
    fmw_memory.h
\endcode

### Modules

A module is a unit of code that performs a well-defined operation, or set of
operations.

It is recommended that modules are written and organized so that the
functionality provided by each module is properly encapsulated, with
well-defined interfaces to other modules. Generally, each module should be
responsible for a single piece of functionality and the overall functionality
required by the firmware should arise from the interaction between modules.

Modules declare themselves as a particular type, depending on the operations
that they perform. The type is informational only as the framework
does not treat a module differently depending on its type, however the types
are used as a guide to the way in which the framework expects operations to be
divided between modules.

The module types are:
- Hardware Abstraction Layer: A module that provides functionality for one or
    more types of hardware device through standardized interfaces that
    abstract away differences between the supported devices. Generally, a HAL
    module will depend on other modules to perform work on its behalf at a lower
    level.

- Driver: A module that controls a specific device, or class of device. Driver
    modules will often implement one or more APIs that are defined by a HAL
    module and the functionality of their device(s) can be used through these
    common interfaces. It is also possible for a driver module to be standalone,
    and to function without an associated HAL module.

- Protocol: Protocol modules implement a protocol and provide one or more APIs
    so that other modules can make use of it. The module will implement any
    protocol-specific handling (arbitration of messaging channels, for example).

- Service: A service module performs work and/or provides functionality that is
    not related to hardware devices. This distinction separates it from the HAL
    and Driver-type modules. A service module does not necessarily have to
    provide services to other modules through APIs, it may perform some work
    that is entirely self-contained.

Every module must declare and define a structure of type *struct fwk_module* and
it is this structure that describes the module within the context of the
framework. The framework uses information from this structure to perform
validation during certain operations such as binding to a module's API.

An example of a module description is given below. This example module defines
itself as a service that provides one API for other modules to use. Because it
provides an API it implements the *process_bind_request* function of the
framework's module interface so that other modules can bind to it. It does not
generate any events nor any notifications.

\code
const struct fwk_module mod_modulename = {
    .name = "Example Module",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .event_count = 0,
    .notification_count = 0,
    .init = modulename_init,
    .element_init = modulename_element_init,
    .bind = modulename_bind,
    .process_bind_request = modulename_process_bind_request,
};
\endcode

#### Module Configuration

For each module that is built as part of a product, a corresponding module
configuration must be provided. This configuration takes the form of a
*struct fwk_module_config* structure that is defined in a configuration file
within the firmware that the module will be built into.

The declaration for the *fwk_module_config* structure is given below:

\code
struct fwk_module_config {
    const struct fwk_element *(*get_element_table)(fwk_id_t module_id);
    const void *data;
};
\endcode

The framework uses the *get_element_table* function pointer to access the table
of elements that the product has provided for the module. If the pointer is
NULL then the framework assumes that no elements will be provided.

Each of the entries in the element table is a pointer to a *struct fwk_element*
structure. Elements are made available to the module during the *element
initialization* stage.

The second member of the structure is an optional void pointer that points to
module-specific configuration data. The format of this configuration data
is defined by the module itself. This data is made available to the module
during the *module initialization* stage.

### Elements

An element represents a resource that is owned or governed by a module. Each
module may have many associated elements, a single element, or no elements at
all.

Element descriptions complement the *module configuration*. There is one
description per element the module contains. In turn, each element description
holds element configuration data, the type of which is defined by the module.
Generally, each element will represent an object that the module interacts
with and/or is responsible for. For example, a driver-type module may have
elements which represent the hardware devices that it controls. Because the
element configuration data is provided as part of a product specification, the
module itself does not need to contain any product-specific data and it can be
written in a way that is as generic as possible.

Elements are defined by a structure containing a pointer to a name string, the
number of sub-elements associated with the element, and a void pointer to data
that is in a module-defined format. The declaration for the *fwk_element*
structure is given below:

\code
struct fwk_element {
    const char *name;
    size_t sub_element_count;
    const void *data;
};
\endcode

### Sub-elements

A sub-element represents a resource that is owned or governed by an element.
Unlike elements, however, sub-elements do not have a structure within the
framework that defines them. Instead, sub-elements are represented by their
indices and/or identifiers alone.

### Indices and Identifiers

Since the framework is designed to be modular there is a need for a standardized
method of identifying and referring to modules, elements, sub-elements, events,
notifications and APIs. The framework defines two components for this purpose:
*indices* and *identifiers*.

#### Indices

Indices are unsigned integers that uniquely identify items within their parent
context. That is, to identify an element, event, notification or API within the
context of a module, a sub-element within the context of an element, or a module
within the context of a firmware.

Module indices are generated for each firmware by the build system and are
placed in the *fwk_module_idx.h* header file.

#### Identifiers

Identifiers are used to uniquely identify items from a global firmware context
(which may either be inside or outside of the items' parent context).

Examples of identifier uses include:

- Identifying a module's elements, events or APIs from the context of another
    module.
- Identifying modules within the context of the firmware as a whole.
- Identifying sub-elements from the context of a module.

Identifiers have a type and this determines the information that is contained
within the identifier. Internally, identifiers always contain the index of a
module and may contain additional indices that identify an item within the
context of that module.

The available identifier types are:

- Module: Consists of the module index alone
- Element: Consists of a module index and an index of an element within the
    module
- Sub-element: Consists of a module index, an index of an element within the
    module, and an index of the sub-element owned by the element.
- API: Consists of a module index and an index of an API provided by the module
- Event: Consists of a module index and an index of an event that may be
    generated by the module
- Notification: Consists of a module index and an index of a notification that
    may be generated by the module.

### APIs

Modules that offer functionality to other modules will do so by defining one
or more Application Programming Interfaces (APIs). Other modules may then bind
to these APIs in order to use the provided functionality. This approach ensures
that interactions between modules are well-defined and that there is a low
degree of coupling across the modules within a firmware.

#### Declaring and Defining APIs

An API is declared in the module's public header as a structure containing one
or more function pointers. Modules may declare multiple APIs, each offering
different functionality.

Within the module's source files each declared API is then defined, with each of
the API's declared function pointers pointing to a function within the module.

#### Advertising APIs

When one module wishes to use an API from another it must first construct an
API identifier so that it can uniquely identify which of the target module's
APIs it intends to use.

The API identifier consists of a *module index* and an *API index*. The former
is used to specify the target module that provides the desired API, while the
latter is used to specify which of the target module's APIs is requested.

A module may offer some affordances so that the *API index* is always
well-defined. For example, the module may provide an enumeration in its public
header that lists the APIs it offers, giving the API indices in a structured
way:

\code
enum mod_modulename_api {
    MOD_MODULENAME_API_A,
    MOD_MODULENAME_API_B,
};
\endcode

Alternatively, the module may define these values individually:

\code
#define MOD_MODULENAME_API_IDX_A    0
#define MOD_MODULENAME_API_IDX_B    1
\endcode

Finally, the module may offer its API identifiers directly using the appropriate
macros to construct the identifiers itself. This approach has the benefit that
modules using the API do not need to create the API identifiers themselves.

\code
static const fwk_id_t mod_modulename_api_id_a = FWK_ID_API_INIT(
    FWK_MODULE_IDX_MODULENAME, MOD_MODULENAME_API_IDX_A);
static const fwk_id_t mod_modulename_api_id_b = FWK_ID_API_INIT(
    FWK_MODULE_IDX_MODULENAME, MOD_MODULENAME_API_IDX_B);
\endcode

### Events

A module may optionally define events - structured messages that are passed from
one module to another. Events are issued by a source towards a target, the
source and target being a module, element, or sub-element.

When an entity receives and processes an event, it may need to respond to the
entity that issued the event. The event contains a *response_requested*
property that indicates whether or not the source entity expects a response to
its event or not. To respond to this event, the receiving entity fills out the
response parameters and the framework issues an event that targets the entity
which issued the original event. The *is_response* property of the event is
used to indicate that the newly-generated event is in response to the original
event.

Events contain a block of memory to store parameters that are used to pass
information between the source and target entity. This memory is intended to be
written to and read through a C structure. The size (bytes) of this space is
defined by *FWK_EVENT_PARAMETERS_SIZE* in fwk_event.h.

#### Notifications

Notifications are used when a module wants to notify other modules of a change
of its own state. A notification is broadcast to a set of modules or elements
that have subscribed beforehand to this notification.

To the framework, a notification is just a special type of event.

Like an event, a notification may require a response from the target entities.
In this case, the same mechanism as events is used.

When a notification is broadcast, the number of subscribers is given to the
notifier by the framework. This information can be used by the notifier, for
instance, to know when all the subscribers have responded to this notification
in the case where a response was required.

## Framework Concepts

This section explains concepts that relate to the framework itself and to the
components that the framework provides, specifically the initialization of the
framework and its components, and the way in which modules and elements are
bound together during this process.

### Phases

During the *pre-runtime phase* the framework directs the execution flow,
configuring components in several stages, outlined in the following section,
until all modules, elements, and sub-elements are initialized, bound together,
and started.

In the *runtime phase* the execution flow is directed primarily by interactions
between modules, by events and by received interrupts. The framework is used to
facilitate, validate, and govern these interactions.

#### Pre-Runtime Stages

The pre-runtime phase is divided into into five stages that occur in a fixed
order:

- Module initialization
- Element initialization
- Post-initialization
- Bind
- Start

Each stage is executed for each module before moving onto the next stage, and
modules are processed in the order they are given in the *BS_FIRMWARE_MODULES*
list.

Once these stages have all been completed the firmware as a whole is considered
to be fully initialized and execution enters the *runtime phase*. The stages are
described in the following sub-sections.

##### Module Initialization

Each module receives its module configuration from the firmware that it will be
built into. The framework invokes the function that the module provides to
satisfy the *init()* function pointer of the framework's module API. During this
stage the module does not have access to any elements and cannot interact with
other modules.

##### Element Initialization

The framework invokes the function that the module provides to satisfy the
*element_init()* function pointer of the framework's module API. This function
is invoked once for each element that is defined in the firmware's element table
for the module.

In the element initialization stage the module receives information about the
elements that have been provided to it via its configuration in the firmware.

During this stage the module may interact with elements as they are
provided to it. It cannot interact with other modules, even if these modules are
referred to in an element's descriptor.

Modules that do not have any elements provided via their configuration in the
firmware are not required to participate in this stage.

**Note:** Participation in this stage is optional if the module has no elements.

##### Post-Initialization

The post-initialization stage is intended to be used by modules to perform any
actions that are required after all its elements have been initialized, yet
before any module-to-module interaction is possible. An example would be some
initialization that requires comparisons between the module's elements as this
is the first stage in which the module has received configuration for all of its
elements.

**Note:** Participation in this stage is optional.

##### Bind

Each module and element has the opportunity to bind to other modules and
elements so that their interfaces can be used in the start stage and during the
runtime phase.

The binding stage is the stage during which modules request access to each
other's APIs. Until modules are bound they have no direct way to call functions
from each other.

**Note:** Participation in this stage is optional.

##### Start

Modules perform initialization that depends on using resources from other
modules and elements, now that binding is complete and these resources are
available. This is the final pre-runtime stage.

**Note:** Participation in this stage is optional.

#### Error Handling

Errors that occur during the pre-runtime phase (such as failures that occur
during module and element initialization, memory management initialization, or
interrupt initialization) are passed from the framework layer into the arch
layer. It is therefore architecture-specific code that determines the ultimate
response to these types of errors.

### Binding

Binding is the process in which modules and elements can form associations with
each other and request access to APIs that are declared by modules within the
framework. Along with issuing events, these should be the two methods by which
a module can invoke functionality from another module.

#### Module-Level and Element-Level Binding

A module or element may bind to another module or element within a module. The
goal is the same - to obtain a pointer to an API that can be used during later
stages.

When attempting to bind to an element within a module (instead of the module
itself) the main difference is that the module that receives and processes the
bind request has the ability to change its behavior depending on the targeted
element. For example, a module that is requesting binding may be permitted to
bind to only a subset of elements within the module processing the request.


#### Processing Binding Requests

When a module receives a binding request it is not required to accept it. The
framework allows a module to reject a binding request if the module's criteria
for correct binding are not met.

For example, a power supply driver may restrict the types of modules that are
allowed to bind to it, so that only a Hardware Abstraction Layer (HAL) module
can utilize the driver.

Alternatively, a driver that is of the *service* type may choose to restrict the
service it provides to only a whitelisted set of modules within the firmware. In
this case the driver module can compare the identifier of the module that is
attempting binding with its whitelist and accept or reject the bind request as
appropriate.

If a binding request is rejected then the framework will consider that an error
has occurred and the binding process as a whole will fail. The handling of this
overall condition is ultimately architecture specific.

### Logging

The framework contains a log component to ensure that logging functionality is
always available and is not tied to the availability of any particular
module. The framework defines and implements the public interface for this
component. Documentation for this interface can be found in fwk_log.h.

To ensure that the framework is platform-independent, the log component relies
on a small set of functions to do platform-specific work like flushing the
buffer and outputting characters. These functions make up the log driver
interface and are forward declared in fwk_log.h:

\code
int fwk_log_driver_init(void);
int fwk_log_driver_putchar(char c);
int fwk_log_driver_flush(void);
\endcode

The framework only implements weakly-linked "stub" versions of these functions
that simply return an error code. It is expected that platform-specific code (a
driver module) will implement these functions properly to complete the logging
functionality of the framework.

If this driver module requires configuration data to be used in the log driver
functions, the usual method of module configuration will not suffice. This is
because the log driver functions could be called before the module receives its
configuration data in the initialization stage of the framework. To allow the
passing of configuration data to this module, the log component in the framework
externally declares a pointer to configuration data: (fwk_log.h)

\code
extern void *fwk_log_driver_config;
\endcode

It is expected that the firmware-specific module configuration code for the
driver module (config_mod_xxx.c) will concretely declare this variable and
initialize it to point to some configuration structure:

\code
struct mod_xxx_fwk_log_config cfg = {
    .x = 1,
    .y = 2,
    .z = 3,
};

void *fwk_log_driver_config = &cfg;
\endcode

The driver module can then access its log framework related configuration data
at any time. It is expected that the driver module performs initialization using
this configuration data in the fwk_log_driver_init() function.
