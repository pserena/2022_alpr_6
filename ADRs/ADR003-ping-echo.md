# ADR 003: Use ping-echo tactic  
According to QA_03, for a network connection problem, a laptop application should detect within 5 seconds. 
To solve this problem, we make the following architecture decision.

## Decision
We will use ‘ping/echo’ tactic to detect network faults.

## Rationale
Alternatives that can detect network faults are monitor and heartbeat. The reason ‘ping/echo’ tactic suits our architecture better than others is as follows. 
Since the laptop and the server are already connected by TCP keepalive, which includes ‘ping/echo’ tactic almost as it is. Also, extra effort and resources 
are required to configure a monitor or heartbeat. Therefore, we plan to apply a ‘ping/echo’ tactic to detect network faults.
	
## Status
Proposed

## Consequences
TBD
