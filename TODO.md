# Current

## 2025-12-25/27 S1 Text Rendering
- [ ] Text rendering utilities
  - [x] Initialize SDL3_ttf and use it to load TTF_Font (with TTF_OpenFont)
  - [ ] Create OpenGL textures with text
    - [ ] Create SDL_Surface with TTF_RenderText_Blended or others
    - [ ] Use SDL_Surface pixels to make an OpenGL texture
  - [ ] Display textures with text on a quad using OpenGL
    - [ ] Make a simple quad mesh that has a diffuse texture and a shader to display that texture
  - [ ] Make a Text struct that contains:
        - String with text
        - Width, height, font size, font reference
        - Whether or not it has an alpha channel
        - Whether or not it uses SDF
        - Texture and shader IDs
  - [ ] Formalize the API using Text struct
  - [ ] Render text using SDF
    - [ ] Enable SDF using TTF_SetFontSDF
      - Surfaces, created with TTF_RenderText_Blended, will contain signed distance value in their alpha channel
    - [ ] Need a custom shader to use this signed distance value
      - <https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf>

# Next
- [ ] Move Input to the engine part. Extract all game specific code and move it to UpdateGame instead.

# Upcoming
- [ ] Skeletal animations
- [ ] Heightmaps for terrain rendering
  - [ ] Check how WARNO/Total War snaps unites to terrain/rotates them on slopes and hills
  - [ ] Make a grid system where units have their Y coordinate set to the Y coordinate of the terrain in the position they are located.
    - WARNO uses coordinates from 0 to 655360 (for fixed point math). They also have coordinate system from 0 meters to 3048 meters for one map unit (can go up to 10 on each axis). Their heightmap is a png of size 1024x1024 for one map unit which is around 2.97 meters per one pixel which then gets smoothed out by interpolation when rendered. Every time the heightmap png is changed, the map has to be baked again to apply the changes.
    - [ ] Also calculate rotation from this grid system (need more research).
- [ ] Audio utils
- [ ] User interface
  - Use Dear Imgui
  - Alt: build custom UI (<https://www.rfleury.com/p/ui-part-1-the-interaction-medium>)
    - Why: good practice
    - Why not: can take too long
- [ ] Raycasting utilities for mouse picking/game logic stuff (Line of sight, shooting projectiles, pathfinding, collision detection, fog of war, lighting, bullet ricochets from surface)
- [ ] Multiplayer
  - Research:
    - <https://forum.godotengine.org/t/how-to-learn-multiplayer-inplementation/43712>
    - <https://gdcvault.com/play/1022195/Physics-for-Game-Programmers-Networking>
    - <https://www.gabrielgambetta.com/client-server-game-architecture.html>
    - <https://developer.valvesoftware.com/wiki/Latency_Compensating_Methods_in_Client/Server_In-game_Protocol_Design_and_Optimization>
  - [ ] Deterministic lockstep (the approach warno uses)
    - Description: send only players commands to each other. Each computer simulates the game with the given inputs and if it's deterministic, it should come out the same way. The simulation happens at a fixed rate (hence the lockstep in the name).
    - Why: low bandwidth. This is pretty much the only way to do a multiplayer RTS game with a lot of units since passing the whole game state snapshot with thousands of units through network is way too long.
    - Why not: all players have to wait for the slowest one which can produce lag. It's also hard to reconnect players/join them to an existing game, because you have to first send a full copy of the game state to that player, then pause all other players so that connecting player can catch up and simulation doesn't go any further. Making the simulation fully deterministic and desync resistant is hard as any little change or data corruption can cause a cascade of changes that diverge game states of the players (butterfly effect).
    - Research:
      - <https://gamedev.stackexchange.com/questions/14776/how-are-deterministic-games-possible-in-the-face-of-floating-point-non-determini>
      - <https://www.gamedeveloper.com/programming/1500-archers-on-a-28-8-network-programming-in-age-of-empires-and-beyond>
      - <https://zoo.cs.yale.edu/classes/cs538/readings/papers/terrano_1500arch.pdf>
      - <http://wiki.factorio.com/Desynchronization>
    - [ ] Need fixed point math for deterministic simulation on different machines (rendering and everything else can still use floats)
      - [ ] Use libfixmath for this (<https://github.com/PetteriAimonen/libfixmath>)
    - [ ] Synchronize connecting players
      - Research:
        - <https://www.reddit.com/r/factorio/comments/1nittpg/multiplayer_experience/>
        - <https://www.reddit.com/r/factorio/comments/adhck1/joining_online_game_taking_a_long_time/>
      - In Factorio server owners can setup "Pause when player joins" so that the player can receive the latest game state and load it without the simulation going further while other players wait for them to load in. This is required for long playthroughs because when the map gets large, it can be hard or impossible for some machines to fast forward the simulation.
      - Alt: use 'fast-forward' mechanism. Player receives latest snapshot, other players still continue the game while connecting one is loading in. Once the outdated game state snapshot is loaded, the server sends all user inputs that happened from that moment and the connecting machine has to fast-forward the simulation by going faster than the main simulation between joined players. In Factorio players usually see 'Catching Up' progress bar. Once the simulation is caught up, newly joined player can now make their own inputs and play the game normally. If the connecting player computer is too slow, they can never catch up in which case "Pause when player joins" is necessary
  - Alt: state sync
  - Alt: send state snapshot

# Log

## 2025-12-19 S0 SessionTaskExample
- [x] High level task1
  - [ ] Lower level subtask
    - [x] Subtask1 of a subtask.
    Multiline subtask description.
    - [ ] Subtask2 of a subtask
- [x] High level task2
  - [ ] Lower level subtask1
    - why: Task pros
    - why not: Task cons
    - alt: An alternative route for a task
  - [ ] Lower level subtask2
