# ADR 006: Use Adapter pattern
With regard to Database, there are various types of DB both in RDBMS and NoSQL DB. As partial match and vehicle information lookup performance must be taken 
into account at the same time, a logic to choose a better solution can be considered to improve license plate query performance.      

## Decision
We will use the “Adapter pattern” tactic (restrict dependency) to guarantee vehicle information lookup performance. 

## Rationale
If we implement adapter pattern, the only thing to do is implementing DBMS adapter interface when DBMS changes. (DBMS APIs that we need are only create/open/close/get/put). And other modules will not be affected. So it can be fast and easy.
	
## Status
Proposed

## Consequences
TBD
