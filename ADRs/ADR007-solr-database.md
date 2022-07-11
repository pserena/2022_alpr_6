# ADR 007: Use Solr with NoSQL Database
ALPR system requires a database to store millions of records about vehicle and supports a partial match to return the best match license plate. 
The database should be simple and scalable in terms of number of DB operations and volume of data, and performance should not be affected too much 
although in the case of partial match. Also, a simple API and data import method should be considered to use the database.      

## Decision
We will use Solr with NoSQL as the read-write data store to persist large amounts of vehicle information in ALPR system. Fuzzy search feature of Solr DB will be used for partial match about vehicle information.ce. 

## Rationale
According to the requirements (store 25 millions of records) and assumptions (write data at once and read data in ALPR function) in our project, many benefits of Document type of NoSQL DB contribute to this decision:
- Solr is simple and easy to accommodate new document types to store large amounts of data.
- Solr has optimized (indexed) for full-text search like fuzzy search, which can be useful for partial match.
- Solr(Post Tool) has lots of ways to index data with various types of file, including json, xml, csv, html, pdf, MS word, plain text and more.
- Solr works better in read-intensive system without read overhead, which has no significant impact on performance.
- Applications can interact with Solr database via REST API (http).
- Client application parses the HTTP responses with json format after creating HTTP requests, which makes it much easier to write client applications.

Rejected alternatives and rationale:

A second Database (Amazon DynamoDB) with NoSQL has been considered to store vehicle information, however, as it doesn’t support full-text search queries like fuzzy matching and regex query, and in order to satisfy partial match for ALPR system, we should combine some technology with like ‘%’ queries. We have tested query latency when using like query in DynamoDB and it takes a very long time to execute queries especially for the case of partial match. It makes us find and choose another appropriate Database to meet the requirements.
	
## Status
	Proposed

## Consequences
ALPR system needs to install Solr with 8.11.2 version on Server side, then import and index 25 
millions of data made by faker. We should be familiar with fuzzy search feature of Solr Database by 
checking the standard Query Parser and API on Solr Ref Guide for query interface and partial match.
