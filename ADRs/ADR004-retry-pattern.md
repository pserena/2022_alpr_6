# ADR 004: Use ‘Retry’ pattern  
To satisfy QA_04, we need to detect when the network connection is reconnected. It is necessary to think about the design to restore the connection once disconnected, 
and we made the following architecture decision.


## Decision
We will use ‘retry’ pattern to resolve the communication issue automatically when detecting network failure.

## Rationale
We have considered using 'retry' tactic that tries repeated reconnection attempts when network failure occurs. 
But, if we use this tactic, a laptop application has to wait and cannot do anything until a response to whether the connection is re-established successfully 
has been delivered. Therefore, for a better user experience, we choose 'circuit breaker' pattern. By delegating the reconnection attempts to the 'circuit breaker', 
the application can do other work.
	
## Status
Proposed

## Consequences
TBD
