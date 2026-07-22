# Bouncing Balls Simulation
A simulation, that has balls bouncing around, colliding with each other and walls. The goal of the project is to make it as optimized as possible.

# Tech stack:
- SFML 3.1.0
- C++17
- CMake

# Benchmark history:
### Version 1
<img width="824" height="259" alt="benchmark1" src="https://github.com/user-attachments/assets/a0bf862d-1154-477f-acf2-3b4300b0ec53" />
- Parameters: circleAmount = 5000, radius = 5.f, speed = 300.f
- 35 FPS
- Notes: checkCollisions is expensive, but getPosition is the bigger red flag. 21% of CPU cycles for a simple getter.
### Version 2
<img width="560" height="281" alt="image" src="https://github.com/user-attachments/assets/30f6b985-20f4-438c-afa1-60e759d3d50d" />
- Parameters: circleAmount = 5000, radius = 5.f, speed = 300.f
- 98fps
- Notes: Replaced SFML Vector2f:s with posX, posY, dirX, dirY and removed usage of getPosition(). Then updated the math accordingly. As a result the program is much more cache friendly.
- Next steps: Now the balls check EVERY other ball, and that could be improved by splitting the window into a grid. A ball inside a certain grid should check collisions only with balls inside the same grid. 
