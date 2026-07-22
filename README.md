# Bouncing Balls Simulation
A simulation, that has balls bouncing around, colliding with each other and walls. The goal of the project is to make it as optimized as possible.

# Tech stack:
- SFML 3.1.0
- C++17
- CMake

# Benchmark history:
### First working version
<img width="824" height="259" alt="benchmark1" src="https://github.com/user-attachments/assets/a0bf862d-1154-477f-acf2-3b4300b0ec53" />
- Parameters: circleAmount = 5000, radius = 5.f, speed = 300.f
- 35 FPS
- Notes: checkCollisions is expensive, but getPosition is the bigger red flag. 21% of CPU cycles for a simple getter.
