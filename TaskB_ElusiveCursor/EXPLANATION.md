# Task B: Elusive Cursor (Concepts First!)

Before diving into the code, let's understand the core concept of Windows Programming: **The Message Loop**.

---

## 🧠 Core Concept: What is the "Message Loop"?
Imagine you are a receptionist at a busy hotel. You sit at your desk, and your only job is to wait for the phone to ring. 
- When the phone rings, you pick it up, hear what the customer wants, and redirect them to the right department.
- Then, you sit back down and wait for the next call.

This is exactly how a Windows program works! It is **Event-Driven**. A Windows program creates an infinite loop called the **Message Loop**. The loop just sits there waiting for Windows to send it an "Event" (like a mouse click, a keyboard press, or a timer going off).

When an event arrives, the Message Loop passes it to a "Receptionist" function called `WndProc` (Window Procedure). The receptionist looks at the event and says, *"Ah, a Timer just went off! Time to move the mouse!"*

---

## 💻 How the Code Works

This program is extremely simple and **hardcoded** to assume a standard `1920x1080` monitor to make the code easy to read.

### Step-by-Step Walkthrough

1. **The Setup (Inside `WinMain`)**
   ```c
   HWND hwnd = CreateWindowExA(0, "SimpleCursorClass", "Hidden", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);
   SetTimer(hwnd, 1, 400, NULL);
   ```
   First, we create an **Invisible Window**. Why? Because Windows only sends messages to programs that have windows! Next, we start a `Timer` that tells Windows to ring our program's phone every `400` milliseconds.

2. **The Message Loop (Inside `WinMain`)**
   ```c
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0) > 0) {
       DispatchMessage(&msg); 
   }
   ```
   This is our receptionist desk. It loops forever, catching messages and sending them (`DispatchMessage`) to our receptionist function.

3. **The Receptionist (Inside `WndProc`)**
   ```c
   if (msg == WM_TIMER) {
       POINT pt;
       GetCursorPos(&pt);
       // ... calculate jump ...
       SetCursorPos(newX, newY);
   }
   ```
   Whenever `WndProc` receives a `WM_TIMER` message (which happens every 400ms), it grabs the current mouse position, calculates a random jump, and forcefully moves the mouse!

4. **Hardcoded Boundaries**
   ```c
   if (newX < 0) newX = 0;
   if (newX > 1920) newX = 1920;
   ```
   To keep things simple, instead of asking Windows what the screen size is, we just hardcode `1920x1080` so the math is super easy to read and explain!
