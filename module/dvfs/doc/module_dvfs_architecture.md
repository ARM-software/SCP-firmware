\ingroup GroupModules Modules
\defgroup GroupDVFS DVFS HAL

# DVFS Module Architecture            {#module_dvfs_architecture_architecture}

## Overview                               {#module_dvfs_architecture_overview}

This module implements a Hardware Abstraction Layer (HAL) API for DVFS
transactions.

The DVFS (Dynamic Voltage and Frequency Scaling) module provides an
interface for modules to get and set the operating frequency level
and limits for a DVFS/performance domain.

The DVFS module defines a driver interface on which it relies to read and set
the voltage and frequency of the clock and PSU devices for each DVFS domain.

A response event notifies the caller of the transaction completion if required.

This module complies with the deferred response architecture.

### DVFS asynchronous operations               {#module_dvfs_architecture_set}

The DVFS module implements the SCMI Performance domain management protocol
`PERFORMANCE_LIMITS_GET`/`SET` and `PERFORMANCE_LEVEL_GET`/`SET` commands.

The DVFS module uses a "fire and forget" mechanism for the `set_level`/
'set_limits` operations. These operations will only report the status of
the initial issuing/queueing of the operation, they do not report the status
of the operation itself.

The request may fail when it is eventually executed but the agent will not
be notified of the failure. A failing request will be retried a number of times
but will be discarded if it cannot be completed within those retries.

Note that a `set_level` operation will not be retried upon failure.

A `set_limits` request will be retried a maximum of `DVFS_MAX_RETRIES` times.

The DVFS `set_level`/`set_limits` operations return to the requestor immediately
the request has been queued for later execution.

If a DVFS/performance domain receives a `set_limits`/`set_level` request which
cannot be initiated immediately it queues the request for later processing.
If there is a request already pending/queued for that domain the new request
will overwrite the pending request. There is only ever a single pending/queued
request with the last level/limits values requested.

## DVFS set frequency/limits flow             {#module_dvfs_architecture_flow}

1) DVFS_set_limits(domain, limits)

    Find the frequency corresponding to the Operating Performance Point (OPP)
    for the requested limits.

    If the DVFS domain is busy go to (3).

    Queue event_set_frequency event for DVFS module using the frequency from
    the OPP.

    The request has been queued, return SUCCESS to agent.

    The request will now be handled asynchronously, go to (4).

2) DVFS_set_level(domain, level)

    Find the frequency corresponding to the Operating Performance Point (OPP)
    for the requested level.

    If the DVFS domain is busy go to (3).

    Queue event_set_frequency event for DVFS module using the frequency from
    the OPP.

    The request has been queued, return SUCCESS to agent.

    The request will now be handled asynchronously, go to (4).

3) DVFS domain request in progress(domain, OPP)

    Is there a request pending already ?
        Yes.
            Overwrite the request OPP with the new OPP.
        No.
            Set the initial values for the pending request.
            number of retries, OPP, etc.
    Exit.


4) DVFS asynchronous set frequency, process_event(domain, frequency)

    event_set_frequency received by the DVFS event handler,
      mod_dvfs_process_event()
    - Retrieve the OPP (voltage, frequency) for the requested frequency.
      This OPP is now the target OPP.
    - Get the voltage from the domain PSU (*) or use the current value if
      available.
        If the current voltage is less than the requested voltage
            Increase the voltage to the OPP voltage (*)
            Set the frequency to the OPP frequency (*)
        Else if the current voltage is higher than the requested voltage
            Set the frequency to the OPP frequency (*)
            Decrease the voltage to the OPP voltage (*)
    - DVFS set_level operation complete, check status.
        - SUCCESS
            Does the domain have a request pending, (see 3 above) ?
                Yes, go to (5) with the frequency from the _pending_
                request.
                No, all done, exit.
        - FAILURE
            Does this request require a retry ?
                Yes.
                    Increment the total number of retries for this request.

                    If the maximum number of attempts for this request has not
                    yet been tried go to (5) using the frequency from the
                    _current_ request.

                    Maximum number of attempts has been reached, discard the
                    request.

                    Does the domain have a request pending, (see 3 above) ?
                        Yes, go to (5) with the frequency from the _pending_
                        request.
                        No, all done, exit.
                No.
                    Discard the current request.
                    Does the domain have a request pending, (see 3 above) ?
                        Yes, go to (5) with the frequency from the _pending_
                        request.
                        No, all done, exit.

(*) Note that this operation may be asynchronous and in that case will follow
the deferred_response architecture.

5) Start next request(domain, frequency)

    Does the domain have a timer configured ?
        Yes.

            Start the timer. When the callback triggers go to (2).

            We use a timer with a short delay to start the next request rather
            than starting the operation immediately. This is configurable for
            each platform. This may be required for platforms which have a
            slow bus, eg I2C, to allow the transaction which is currently in
            progress to cpmplete before we initiate the next request.

        No.
            Go to (2). Start the next request immediately.
