# Bouncing Balls Simulation
A simulation, that has balls bouncing around, colliding with each other and walls. The goal of the project is to make it as optimized as possible.

# Tech stack:
- SFML 3.1.0
- C++17
- CMake

## Benchmark History

### Version 1: Brute-Force & Object-Oriented

![Benchmark 1 CPU Hotspots](https://github.com/user-attachments/assets/a0bf862d-1154-477f-acf2-3b4300b0ec53)

* **Parameters:** `circleAmount = 5000`, `radius = 5.f`, `speed = 300.f`
* **Performance:** 35 FPS 
* **Notes:** While `checkCollisions()` is heavy, `getPosition()` turned out to be a massive bottleneck, consuming over 21% of CPU cycles due to object-oriented getter overhead and cache misses.

---

### Version 2: Data-Oriented Design (Struct of Arrays)

![Benchmark 2 CPU Hotspots](https://github.com/user-attachments/assets/30f6b985-20f4-438c-afa1-60e759d3d50d)

* **Parameters:** `circleAmount = 5000`, `radius = 5.f`, `speed = 300.f`
* **Performance:** 98 FPS
* **Notes:** Replaced SFML `Vector2f` objects with std::vectors (`posX`, `posY`, `dirX`, `dirY`) and completely removed the usage of `getPosition()`. The program is now cache-friendly.
* **Next Steps:** Every circle currently checks every other circle ($O(N^2)$). This will be optimized next by implementing a Spatial Grid so circles only check collisions against neighbors inside their local grid cell.

### Version 3: Spatial Grid upgrade
![Benchmark 3 CPU Hotspots](https://github.com/user-attachments/assets/5c13d735-e236-4b81-92e7-2f4d0d199dfb)
* **Parameters:** `circleAmount = 5000`, `radius = 5.f`, `speed = 300.f`
* **Performance:** 880 FPS
* **Notes** Implemented spatial grid based collision checking -> Particles are grouped into cells, only checking collisions inside their own cells and neighboring cells. Used a contiguous linked list as the data structure to keep the program cache friendly and prevent memory fragmentation.
* **Next Steps** Maybe sending a vertex array to pack the draw instructions instead of sending each one separately.
