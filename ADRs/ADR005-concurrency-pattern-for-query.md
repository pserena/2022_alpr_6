# ADR 005: Use introduce concurrency pattern for query request
In QA_05, we suggested the maximum response time for the longest request as 0.1 second. Since the suggested time must be satisfied even in a situation where several 
requests arrive almost simultaneously, we decided on the following architecture.


## Decision
We will use the “Introduce Concurrency” tactic to guarantee vehicle information lookup performance.

## Rationale
In order to improve detection time, it’s necessary to consider vehicle information lookup performance and increase it, especially in the case of multiple requests. Thus, a tactic like ‘Introduce concurrency' is selected to reduce waiting time and then improve detection time.
	
## Status
Proposed

## Consequences
According to experiment 2, it is more efficient when we are using concurrency (Especially for the case of multiple clients.)

So we will use “introduce concurrency” to fix it.
