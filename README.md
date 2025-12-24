# Traffic Simulation Engine (C++/SFML)

A real-time 2D traffic simulation built to demonstrate physics-based entity movement, state machine logic, and object-oriented architecture in C++.

https://github.com/user-attachments/assets/cecc0649-a17b-493f-ab74-4090b0af4e6a


## Overview
This project simulates autonomous agents navigating a traffic grid using Newtonian mechanics. Unlike grid-based logic, movement is calculated via continuous physics integration, requiring agents to manage momentum, braking distances, and collision avoidance in real-time.

### Tech Stack
* **Language:** C++ (ISO C++17)
* **Dependencies:** SFML 2.6.1 (Graphics/Windowing)
* **Environment:** Visual Studio 2022

## Implementation Details

### Physics Engine
Custom implementation of linear kinematics using Euler integration. Movement is not tile-based; it is continuous and frame-rate independent (utilizing Delta Time).
* **Inertia:** Vehicles do not stop instantly. Braking applies a negative force relative to mass.
* **Collision Avoidance:** Agents calculate the relative distance to the leading vehicle to determine required braking force.

### Object-Oriented Architecture
Uses a polymorphic entity system to handle different vehicle behaviors:
* `Vehicle` (Base): Handles core physics integration and rendering.
* `Car` (Derived): Low mass, high braking power, standard compliance behavior.
* `Truck` (Derived): High mass (high momentum), lower braking power. Logic overrides standard stopping behavior at yellow lights due to stopping distance constraints.

### Finite State Machine (FSM)
Traffic control is managed via a state machine (`RED` -> `GREEN` -> `YELLOW`).
* Agents query the environment state every frame.
* Implemented behavior checks where `Truck` entities will ignore `YELLOW` states if momentum is too high to stop safely.

### Memory Management
* Dynamic allocation using `std::vector<Vehicle*>`.
* Manual garbage collection loop to identify and `delete` out-of-bounds entities to prevent memory leaks.

## Build Instructions

1.  Clone the repo.
2.  Open `.sln` in Visual Studio 2022.
3.  Ensure SFML Include (`/include`) and Library (`/lib`) directories are linked in Project Properties.
4.  Copy SFML `.dll` files to the build directory (`/x64/Debug`).
5.  Build and Run.
