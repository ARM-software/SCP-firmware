# SCP-Firmware Threat Model

## Overview

The SCP provides services to agents in the system. The agents might be the
operating system or the PSCI agent running on the application processor. Other
agents in the system could also generate requests for resources that the SCP
controls directly. Examples of such agents might be a modem subsystem in a
mobile SoC or a management function in a server SoC. The SCP reconciles requests
from all agents, managing the availability of shared resources and power
performance limits according to all constraints. The SCP has a minimum set of
resources, including local private memory, timers,interrupt control, and
registers for system configuration, control and status.

In a system comprising multiple agents, one trusted agent typically has elevated
privileges to configure and control the access rights of other agents in the
system. PSCI agent generally is the trusted agent. The platform is responsible
for securing the privileged agent.


## SCP-Firmware Data Flow Diagrams

![SCP Firmware Data Flow Diagram](threat_model/Threat_model.drawio.svg)

The SCP Firmware provides system agents like OSPM, PSCI, MCP, LCP, and RSS
control over hardware resources such as power domain gating, voltage supplies,
clocks, sensors and counters. The scope of the threat model assets are the SCMI
resources including devices and protocols. The trust boundary is between the
SCMI Agents and the SCP.

## Assumptions

Agents send requests over a private transport channel. A typical channel is a
shared memory or MHU. The transport channel is dedicated to a specific agent.
An agent should not be able to access another agent's channel.
Such privacy is enforced by an external entity such as the operating system,
trusted firmware or the hardware. The SCP-Firmware cannot enforce or determine a
violation of this rule.

## Resource Permissions:

In a system with multiple agents, the SCMI protocol provides commands that allow
a trusted agent to configure the access permissions of other agents. An agent
should not be able to discover resources and commands it cannot access.
Only trusted agents can access the Base Protocol commands to configure
agent-specific permissions. The PSCI agent is the only trusted agent in the
system, and it's the only agent that can configure resource permissions.

## SCP-Firmware STRIDE Threat Analysis

### Spoofing:

According to the SCMI specification, each agent is identified by its dedicated
transport channel via the SCP. It is important to note that the protection of
the security and integrity of the channel is not included in the scope of SCP
Firmware. Therefore, this threat is not considered by the SCP threat model.

### Tampering:

The threat model assumes that hardware means protecting against tampering. Each
agent has a dedicated transport channel which another agent cannot  and should
not access. Also, internal SCP registers and memory are not accessible by
external agents.

### Repudiation:

This threat is more applicable to systems where the history of commands sent by
an agent is essential and saved in a log buffer. For example, if an agent sends
a command, then denies this fact. This threat is not relevant or considered a
threat to SCP-Firmware.

### Denial of Service:

Agents send requests over a transport channel. A typical channel is MHU doorbell
or fastchannel. These channels are dedicated to a specific agent. Although an
attacker agent can't access another agent channel, continuously sending requests
will likely keep the SCP busy and starve the other agents, resulting in a denial
of service. Another possible scenario is when an attacker agent repeatedly sends
requests faster than the SCP can dequeue and process. This situation will fill
the queue and result in a denial of service.

The SCP should protect against flood attacks, which makes it unavailable to
legitimate agents.

### Information Disclosure:

The SCP should protect against disclosing sensitive sensor data using device
resource permissions. Access to all counter data is not allowed. SMCF should not
be externally accessible.

### Elevation of Privilege:

The SCP should not allow an OSPM or MCP agent to change the state of a core
power domain. Only a PSCI agent is authorized.

An SCMI agent should not be able to access or modify the permissions of a
protocol, command or device if it is not permitted to do so. Only a trusted
agent should be able to modify access permissions of Secure or Root platform
resources. A non-trusted agent cannot change Root or Secure platform resource
access permissions.

## Software security requirements

| ID  | Asset | Attacker | Attack type | Attack description |
| --- | :---: | :------: | :----------------: | :--------- |
| 01 | A01 | TA01 TA03 | E | Only PSCI Agent is allowed to modify the core power domain state |
| 02 | A01 A02 A03 A04 | TA01 TA03 | E | An SCMI protocol should only be accessible to agents with the correct SCMI permissions |
| 03 | A01 A02 A03 A04 | TA01 TA03 | E | An SCMI command should only be accessible to agents with the correct SCMI permissions |
| 04 | A01 A02 A03 A04 | TA01 TA03 | E | A device should only be accessible to agents with the correct SCMI permissions |
| 05 | A05 | TA01 TA03 | E | Only trusted agents are authorized to modify resource permissions |
| 06 | A01 A02 A03 A04 | TA01 TA02 TA03 | D | It is important to prevent any attempt by agents to flood attack the SCP-Firmware, as this could lead to the starvation of other agents |
| 07 | A03 | TA01 TA02 TA03 | I | It is essential to ensure that agents do not have access to sensitive sensor data or counters |


## Software security mitigations

| ID  | Asset | Attacker | mitigation description |
| --- | :---: | :------: | :--------- |
| 01 | A01 | TA01 TA03 | The SCMI power domain module should only accept the set state command from the PSCI agent |
| 02 | A01 A02 A03 A04 | TA01 TA03 | The SCMI resource permissions module should reject requests to a specific protocol if the agent is not authorized to use it |
| 03 | A01 A02 A03 A04 | TA01 TA03 | The SCMI resource permissions module should reject requests to a specific command for a particular agent if the agent is not allowed to use this command |
| 04 | A01 A02 A03 A04 | TA01 TA03 |The SCMI resource permissions module should reject requests to a specific device if the agent is not authorized to access this device |
| 05 | A05 | TA01 TA03 | The SCMI resource permissions module should reject requests to modify access permissions if the agent is not trusted |
| 06 | A01 A02 A03 A04 | TA01 TA02 TA03 | The SCMI module should service requests from agents in a round-robin fashion to avoid starving other agents |
| 07 | A03 | TA01 TA02 TA03 | Platform configuration should not include sensitive sensors and counters in the SCMI list of devices exposed to agents |
