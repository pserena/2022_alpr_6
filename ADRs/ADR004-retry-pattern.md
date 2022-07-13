# ADR 004: Use ‘Retry’ pattern  
To satisfy QA_04, laptop application needs to detect and resolve automatically when the network connection is reconnected. It is necessary to think about the design to recover the connection once disconnected, and we made the following architecture decision.


## Decision
We will use ‘retry’ pattern to resolve the communication issue automatically when detecting network failure.

## Rationale
We have considered using 'curcuit breaker' tactic when network failure occurs. However, in that case, care must be taken in choosing timeout values. If the timeout is too long, then unnecessary latency is added. But if the timeout is too short, it can lower the availability and performance of these services.   

Therefore, we use retry tactic to detect repeated reconnection attempts when network failure occurs until the connection is re-established successfully. 
Then user's login(user's ID and PW will be saved on memory) will be reconnected and communication between client and server will be recovered. 
	
## Status
Proposed

## Consequences
TBD
