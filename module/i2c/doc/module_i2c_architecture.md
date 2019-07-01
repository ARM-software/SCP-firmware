\ingroup GroupModules Modules
\defgroup GroupI2C I2C HAL

Module I2C Architecture
=======================

# Overview                                   {#module_i2c_architecture_overview}

This module implements a Hardware Abstraction Layer (HAL) API for I2C
transactions.

# Architecture                           {#module_i2c_architecture_architecture}

The I2C module provides an interface for modules to transmit data through an I2C
bus, to receive data from an I2C bus and to perform a transmission followed by a
reception on the I2C bus.

The I2C module defines a driver interface on which it relies to transfer/receive
data to/from the bus.

A response event notifies the caller of the transaction completion.

# Restriction                             {#module_i2c_architecture_restriction}

The following features are unsupported. Support may be added in the future.

- Acting like a slave.
- Concurrent accesses over the bus.
- 10-bit slave addressing.

# Flow                                           {#module_i2c_architecture_flow}

## Asynchronous API

The following schematic describes the transaction flow for an I2C master
asynchronous transmission. The flow for an asynchronous reception is similar.

    Client             I2C         I2C Driver     I2C ISR (Driver)
      |                 |               |               |
      |    transmit_    |               |               |
     +-+   as_master    |               |               |
     | +-------------->+-+              |               |
     | |               | +- - +         |               |
     | +<--------------+-+    |process_ |               |
     +-+                |     |event E1 |               |
      |                 |     |         |               |
      |                +-+<- -+         |               |
      |                | |  transmit_   |               |
      |                | |  as_master   |               |
      |                | +------------>+-+              |
      |                | +<------------+-+              |
      |                +-+              |               |
      |                 |               |  transaction +-+
      |                 |               |  _completed  | |
      |                +-+<-------------+--------------+ |
      |            +- -+ |              |              | |
      |  process_  |   +-+--------------+------------->+ |
      |  event E2  |    |               |              | |
      |            +- >+-+              |              +-+
      |                | |              |               |
     +-+<- - - - - - - +-+              |               |
     +-+  process_      |               |               |
      |   event R1      |               |               |

    E1   : Request event
    E2   : Request completed event
    R1   : Response to the request event E1
    ---> : Function call/return
    - -> : Asynchronous call via the event/notification interface

The client calls *transmit_as_master* API of the I2C module.
This function creates and sends the targeted I2C device request event which
defines the targeted slave on the bus and the data to be transmitted. When
processing the request event, the I2C module initiates the transfer by
programming the I2C controller through the *transmit_as_master* API of the I2C
driver.

An interrupt is asserted when the I2C transaction either completes or encounters
an error. The I2C ISR calls the *transaction_completed* API of the I2C HAL
module which sends a response event to indicate that transaction completed to
the client.

In the case of *transmit_then_receive_as_master*, the I2C HAL does not send a
event at the end of the transmission. Instead it starts the reception by calling
*receive_as_master* driver function. The event is then sent when the reception
has completed.
