\ingroup GroupModules Modules
\defgroup GroupMetrics Metrics service

# Metrics

Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.

## Overview

The metrics analyzer module is a service module that represents the analysis
phase from the power management flow.

This module is responsible for collecting the limits from the power limit
providers and then determining the final power limits for system domains.

The system domains are bound by the lowest power limit metric.

## Data Flow

This module **consumes** `power limits` from multiple providers.

Limits providers must implement `get_limit` API which returns the power_limit
for the givin domain.

The following diagram shows the data flow for each domain (domain view)

```mermaid
flowchart LR
    classDef Providers fill:#006666
    classDef Analyzer fill:green
    classDef Consumer fill:#C3751B
    P0(Domain_x_Limit_Provider_0):::Providers -->|power_limit| Metrics_Analyzer_Domain_x:::Analyzer
    P1(Domain_x_Limit_Provider_1):::Providers -->|power_limit| Metrics_Analyzer_Domain_x:::Analyzer
    Ps(...):::Providers == power_limits==>Metrics_Analyzer_Domain_x:::Analyzer
    Pn(Domain_x_Limit_Provider_m):::Providers -->|power_limit| Metrics_Analyzer_Domain_x:::Analyzer
    Metrics_Analyzer_Domain_x:::Analyzer -->
    |power_limit| c{{consumer_x}}:::Consumer
```

> Where `x` belongs to [0,`N`] and `N` is the number of domains.

The relationship between the providers and a domain is many to one,
while each domain has only one consumer.

## Operation

When the analyze API is called, for each domain, the power limit of each metrics analyzer domain is aggregated from the list of power limits collected
per metric.

```mermaid
sequenceDiagram
  participant Coordinator
  participant Provider_0
  participant Provider_1
  participant Provider_m
  participant Metrics_Analyzer
  participant Consumer
  Coordinator->>Metrics_Analyzer: analyze()
  loop for each domain
    Metrics_Analyzer->>Metrics_Analyzer: Set aggregate limit to max
    Metrics_Analyzer->>Provider_0: get_limit(domain_id, &power_limit)
    Provider_0--)Metrics_Analyzer: status
    Metrics_Analyzer->>Metrics_Analyzer: Update aggregate limit
    Metrics_Analyzer->>Provider_1: get_limit(domain_id, &power_limit)
    Provider_1--)Metrics_Analyzer: status
    Metrics_Analyzer->>Metrics_Analyzer: Update aggregate limit
    Metrics_Analyzer->>Provider_m: get_limit(domain_id, &power_limit)
    Provider_m--)Metrics_Analyzer: status
    Metrics_Analyzer->>Metrics_Analyzer: Update aggregate limit
    opt if evaluation is necessary
        Metrics_Analyzer->>Metrics_Analyzer: evaluate_minimum(domain_id)
    end
    Metrics_Analyzer->>Consumer: set_limit(domain_id, power_limit)
    Consumer--)Metrics_Analyzer: status
  end
  Metrics_Analyzer--)Coordinator: status
```
