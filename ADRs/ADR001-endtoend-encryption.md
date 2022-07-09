# ADR 001: Protect data end-to-end with encryption 
ALPR system consists of laptop application and server application, and provides secure communication between the laptop application 
and to the backend license plate database lookup system. Therefore, how to protect data with encryption to ensure secure communication 
is a challenge. Encryption can protect data in transit point to point and is widely used, however in this project, the backend server 
will be assessed from the police vehicle via network. An encryption algorithm for protecting data end to end should be considered.    

## Decision
We will use end-to-end encryption.

## Rationale 
Point-to-point encryption (ex: TLS) is still vulnerable to Man in the Middle Attacks.
To address this risk, AES is used for end-to-end encryption, which is one of the most popular
cryptographic algorithms that we are familiar with.

End-to-end encryption has the advantage that encrypted data is protected between sender and receiver. 
At present, there is no known practical attack that would allow someone without knowledge
of the key to read data encrypted by AES when correctly implemented.

※ What is MITM attack?
A man in the middle (MITM) attack is a general term for when a perpetrator positions himself in a conversation between a user 
and an application—either to eavesdrop or to impersonate one of the parties, making it appear as if a normal exchange of information is underway.
	
## Status
Concluded

## Consequences
AES, one of the most popular cryptographic algorithms that we are familiar with, we tested the performance of the algorithm to see the feasibility.
- encryption average time : 0.437ms 
- decryption average time : 0.626ms

There is no problem both for secure communication and performance.
