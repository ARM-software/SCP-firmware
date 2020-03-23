Deferred Response Architecture
==============================

# Overview                            {#deferred_response_architecture_overview}

This document describes at high level the typical interactions between the
following actors:
- A client requesting some data/operations from a module.
- A HAL module defining an interface to request operations.
- A driver interacting with the hardware and bound to the HAL.
This document particularly focuses on the interactions when the requests are
deferred and thus acknowledged as pending.

# Architecture                    {#deferred_response_architecture_architecture}

## Client-HAL Interaction           {#deferred_response_architecture_client_hal}

A call to an HAL API can be handled asynchronously by the HAL module. The call
to the HAL API returns with one of the following status code:
- FWK_SUCCESS: the requested operation has completed. The data requested, if
any, is available.
- FWK_PENDING: the call has been acknowledged and the response to the request
will be sent subsequently via a *HAL_module_event_id_request* response event.
In this case, the client must handle the response event.
- FWK_E_X: the request failed.

## Driver-HAL Interaction           {#deferred_response_architecture_driver_hal}

When the HAL calls the driver, it acts in different manners depending on the
status returned by the driver:
- FWK_SUCCESS: the driver has successfully completed the request and the
value/operation has completed immediately. The HAL returns the data, if any, to
the client.
- FWK_PENDING: the driver has deferred the requested operation and will provide
the response later through the *driver_response_api*. In this case, the HAL
sends an *HAL_module_event_id_request* event to itself, the response of which
will be sent to the client when the operation will be completed.
- FWK_E_X: the request failed.

## Note                                   {#deferred_response_architecture_note}

A call to an HAL API is done as part of the processing of an event by a given
entity and, when processing pending requests, this entity is the target of the
response event. In order for the client to receive the response event, the
client is required to call the HAL API within the context of processing an event
targeting itself. In other words, the caller may be required to send an event
targeting itself first and then call the HAL module when processing such event.

# Flow                                    {#deferred_response_architecture_flow}

Below is a typical execution flow for a call to the HAL when the driver defers
the requested operation.

       Client            HAL Module           Driver           Driver ISR

          |                  |                  |                  |
          |                  |                  |                  |
         +++     get/set     |                  |                  |
         | +--------------->+++                 |                  |
         | |                | |     get/set     |                  |
         | |                | +--------------->+++                 |
         | |                | |                | |                 |
         | |                | +<---------------+++                 |
         | |                | |       PENDING   |                  |
         | |                | +-----+           |                  |
         | |                | |     |           |                  |
         | |                | +<----+ PE1       |                  |
         | |                | |                 |                  |
         | +<---------------+++                 |                  |
         +++        PENDING  |                  |                  |
          |                  |                  |                  |
          |         E1>>>>>>+++                 |                  |
          |                 | |                 |                  |
          |                 +++                 |                  |
          |                  |                  |    DRV_RESP     +++
          |                 +++<----------------------------------+ |
          |                 | |                 |                 | |
          |                 | +-----+           |                 | |
          |                 | |     |           |                 | |
          |                 | +<----+ PE2       |                 | |
          |                 | |                 |                 | |
          |                 +++---------------------------------->+ |
          |                  |                  |                 +++
          |                  |                  |                  |
          |         E2>>>>>>+++                 |                  |
          |                 | +-----+           |                  |
          |                 | |     |           |                  |
          |                 | +<----+ PE3       |                  |
          |                 +++                 |                  |
          |                  |                  |                  |
         +++<<<<<<RE         |                  |                  |
         | |                 |                  |                  |
         | |                 |                  |                  |
         +++                 |                  |                  |
          |                  |                  |                  |

    LEGEND
    get/set  : A request to the HAL interface
    PENDING  : The call returns with the FWK_PENDING status code
    PEx      : An event is sent (put_event)
    E1       : Processing event E1 (request event)
    DRV_RESP : driver_response_api
    E2       : Processing event E2 (request complete event)
    RE       : Processing the response event
    ----->   : Direct call/return API
    >>>>>>   : Asynchronous call via the event interface

    EVENTS CORRESPONDENCE
    PE1 > > > E1
    PE2 > > > E2
    PE3 > > > RE

A client calls *get/set* HAL module API which directly calls the driver.

The driver cannot do the operation immediately, it returns FWK_PENDING.

The HAL module sends a *REQUEST* event (PE1) to itself and returns FWK_PENDING
to the client. The request event contains all the information needed for the
deferred response.

The HAL receives and processes the *REQUEST* event E1, and it stores the event's
cookie and delays its response.

When the requested *get/set* operation is completed, the driver calls the
*driver_response_api* providing the result of operation.
In this call, the driver sends a *REQUEST_COMPLETE* event (PE2) for the HAL
to defer post-processing and to provide the result of the operation.

Then, the HAL module receives and processes the *REQUEST_COMPLETE* event E2,
retrieving the result of the operation from the event E2's parameters. At the
same time the HAL get back the response event that was delayed, and sends the
event (PE3) containing the result of the operation.

Finally, the client receives and processes the *HAL_module_event_id_request*
response event RE sent by the HAL module.
