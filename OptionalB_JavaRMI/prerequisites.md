# Context for Antigravity AI (Windows Boot Session)

Hello Antigravity! When you read this, I am currently booted into my Windows machine. 

## The Mission
I am working on my CSC413/CSE315 Operating Systems Spring 2026 Programming Assignment (refer to `OS Projects- Spring2026.pdf` in the root folder). I have already completed the mandatory section. 

**Your current objective is to help me complete Optional Section B (Java RMI Extension).**

## Optional B Details
- **Goal:** Take a previous Object-Oriented Programming (Java GUI) project and extend it using Java RMI (Remote Method Invocation).
- **Requirements:**
  1. Offload some methods (either security-sensitive or compute-intensive) from the client to a remote RMI Server.
  2. The code must not use modern frameworks like Spring; it must use native `java.rmi.*`.
  3. We must be able to justify *why* those specific methods were moved to the server.

## Next Steps for You (The AI)
1. Ask me to provide or upload the `.java` files from my previous Java GUI project.
2. Once I provide them, analyze the code and identify 2-3 methods that are good candidates to be moved to a remote server.
3. Write an interface `IMyService.java` (extending `Remote`).
4. Write the server implementation `MyServiceImpl.java` (extending `UnicastRemoteObject`).
5. Write a `Server.java` to start the RMI Registry on port 1099.
6. Refactor my GUI code to use `Naming.lookup()` to call the remote methods.
7. Give me step-by-step instructions on how to compile and run both the server and the client in separate terminals.
