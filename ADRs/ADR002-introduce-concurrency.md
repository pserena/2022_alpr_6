# ADR 002: Use introduce concurrency tactic for output frame rate  
The laptop application of the ALPR system should be able to decode the input video, perform license plate recognition, receive vehicle information of the recognized license plate from the server, display the video on the screen, and store the output video. And the output video should maintain the performance of 25 fps. Maintaining performance while performing all the tasks described above is not an easy problem. So the architecture decision we have chosen to maintain performance is as follows.

## Decision
We make the decision to choose ‘introduce concurrency’ which is one of the performance tactics to maintain a frame rate of at least 25fps.

## Rationale
According to the experiment results of checking ALPR processing latency, concurrency can be introduced by
creating additional threads to process different sets of activities. The alternative tactic is ‘increase resources’ to improve performance, however the tradeoff is cost. Therefore, we choose ‘introduce concurrency’ as there is not enough budget.
	
## Status
Proposed

## Consequences
According to experiment 1 and 4, clients can send query within about 40ms(average) - It means that frame rate can be handled at least 25 FPS. 
According to experiment 2, we need time to get the response  from DB on the server side. (up to about 40ms).
     
So we need to separate the threads
1. recognize and send a query to server
2. receive the query response and handle it.
