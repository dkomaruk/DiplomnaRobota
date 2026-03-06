# Current

## 2026-03-06 S8 More Stuff
- [ ] Multithreaded asset loading for faster iteration time
  - Load assets in parallel
- [ ] Terrain optimization (chunking, tesselation and LOD)
- [ ] Forest, grass, bushes rendering (instancing, impostors at high distance, LOD)
  - Load meshes, impostors, grass textures and so on
  - Distribute them on the terrain (jitter grid, blue noise, perlin noise)
  - Draw in one call

# Next
- [ ] Map Editor
  - Ability to place individual items on the terrain
  - Place fillers for vegetation and random items
  - Draw on the terrain into a splat map
  - Save heightmap, other textures, placed object positions, object descriptors into one file
- [ ] Multithreading
  - Update Entities in parallel
- [ ] Manual Memory Management
  - Arenas, general purpose allocators
- [ ] Entities
- [ ] Game User Interface
  - Alt: build custom UI (for game) (<https://www.rfleury.com/p/ui-part-1-the-interaction-medium>)
    - Why: good practice
    - Why not: can take too long
- [ ] Pathfinding
  - A*, graphs
- [ ] Enemy AI
- [ ] PBR
- [ ] Fixed point math for simulation

# Upcoming
- [ ] Further Text Improvements
  - [ ] Make a single draw call for all visible dynamic text instead of making a draw call per instance of dynamic text
  - [ ] Render text using SDF. This approach allows rendering low resolution glyph textures at high quality by using distances instead of displaying already rasterized letters.
    - [ ] Enable SDF using TTF_SetFontSDF
      - Surfaces, created with TTF_RenderText_Blended, will contain signed distance value in their alpha channel
    - [ ] Need a custom shader to use this signed distance value
      - <https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf>
- [ ] Improve Terrain Rendering
  - [ ] Check how WARNO/Total War snaps unites to terrain and how it orients them on slopes and hills
    - WARNO uses coordinates from 0 to 655360 (for fixed point math). They also have coordinate system from 0 meters to 3048 meters for one map unit (can go up to 10 on each axis). Their heightmap is a png of size 1024x1024 for one map unit which is around 2.97 meters per one pixel which then gets smoothed out by interpolation when rendered. Every time the heightmap png is changed, the map has to be baked again to apply the changes.
    - [ ] Also calculate rotation from this grid system (need more research).
- [ ] Audio Utils
- [ ] Polish Particle System
  - [ ] Add particle system demo
  - [ ] Ability to add new emitters at some position. Also select and drag existing ones
  - [ ] Improve fade in/out behaviour (fix popping in and out of existence, make it customizable)
  - [ ] Overdraw issue: <https://community.khronos.org/t/huge-performance-drop-when-rendering-60-tris-on-screen-overdraw/108095>
  - [ ] Point sprites?
- [ ] Raycasting Utilities (for mouse picking/game logic stuff like line of sight, shooting projectiles, pathfinding, collision detection, fog of war, lighting, bullet ricochets from surface and so on)
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

## 2026-02/03 S7 Stuff
- [x] Move Input to the engine part. Extract all game specific code and move it to UpdateGame instead.
- [x] Basic Editor Interface
  - Use Dear Imgui (for editor)
- [x] Raycasting, ray AABB and AABB frustum intersection tests

## 2026-02-05/07 S6 Model transform hierarchy
- [x] Models need to keep the node hierarchy in order to transform separate parts of the model (and make children of that part move along)

## 2026-01-22/27 S5 Skeletal Animation
- [x] Skeletal Animation
  - [x] Use Assimp to load meshes, bones and animations
  - [x] Assign bone ids and weights to vertices before uploading them to the GPU
  - [x] Update animations
    - [x] Each frame advance the current animation by deltaTime
    - [x] Go through the node hierarchy from the top
    - [x] If the node is animated, interpolate between current and next keyframes (position, rotation, scale) and create a transformation matrix. Otherwise a default node transform is used (which is a relative position from its parent).
    - [x] Join current node transformation matrix and accumulated parent transformation matrix to get global transformation matrix at that node
    - [x] If node is a bone, the global transform is multiplied with the inverse bind pose matrix to move from model space into bone space. This resulting matrix is then associated with this bone and applied to vertices influenced by it in the vertex shader (if a vertex is influenced by more than one bone, the matrices of those bones are blended together using bone weights as a blend factor)
    - [x] Pass the final matrix of the node to its children to repeat the same process

## 2026-01-17/22 S4 Terrain Rendering
- [x] Terrain Rendering
  - [x] Load a heightmap texture, make a mesh out of it
  - [x] Sample heightmap at normalized world coordinates to get Y coordinate at XZ point
    - [x] Use bilinear interpolation to sample the coordinate between four pixels in the heightmap
  - [x] <https://gamedev.stackexchange.com/questions/24572/how-does-terrain-following-work-on-height-map?rq=1>

## 2026-01-08/17 S3 Particle System
- [x] Particle System
  - [x] Gradient generator for color ramp during particle lifetime
  - [x] Add particle system settings preset saving/loading
  - [x] Curve editor for controlling parameters over lifetime of the particle
  - [x] Transparency issue: <https://community.khronos.org/t/proper-rendering-for-particles/44223/4>

## 2025-12-29/2026-01-08 S2 Improved Text Rendering
- [x] Improve text rendering
  - [x] Add dynamic text rendering using quads for each glyph instead of making a texture for the whole string of text
        - Right now for each piece of text a separate texture is created which is then rendered on a simple quad with orthographic and model matrices applied to its coordinates in vertex shader to move it into correct position and give it correct size.
        - This is bad because it means mutliple draw calls for all texts on the screen. Also, if you want to change this text then you have to create another texture and send it to the GPU.
        - It would be better to create a texture atlas with all needed glyphs. Each character would contain its font metrics and uv coordinates into that atlas. When a string of text is needed, a new mesh is created from multiple quads for each letter. Position of each vertex in those quads is calculated based on the position of the text + different offsets based on glyph metrics (bearing, width/height, advance), each vertex also has uv coordinates into text atlas for its glyph.
        - With this approach it's not needed to create a texture for new text all the time. You can also combine all visible text into a single mesh and have a single draw call (harder to manage the memory of this mesh when text changes).
        - Here are some helpful references:
          - <https://www.reddit.com/r/opengl/comments/15s4h0d/strategies_for_efficient_text_rendering/>
          - <https://learnopengl.com/In-Practice/Text-Rendering>
          - <https://www.youtube.com/watch?v=S0PyZKX4lyI>
  - [x] Fix dynamic text glyph positioning being slightly different from static text (mosly fixed)
        - The issue seems to be on the SDL3_ttf side. It's better to directly use FreeType library
        - !!! Seems like it's just the problem with TTF_RenderText_Blended. When rendered with blending turned off, the texture quad is taking the same amount of space as dynamic text, but static text has some empty space at the end.
        - <https://forums.libsdl.org/viewtopic.php?p=37700>
  - [x] Don't bake in glyph positions into vertex data. Instead store offset from text origin in the vertex position and pass model matrix that translates glyph to the correct position with offset

## 2025-12-25/28 S1 Text Rendering
- [x] Static text rendering utilities
  - [x] Initialize SDL3_ttf and use it to load TTF_Font (with TTF_OpenFont)
  - [x] Create OpenGL textures with text
    - [x] Create SDL_Surface with TTF_RenderText_Blended
    - [x] Use SDL_Surface pixels to make an OpenGL texture
  - [x] Display textures with text on a quad using OpenGL
    - [x] Make a simple quad mesh that has a texture and a shader to display that texture
  - [x] Make a Text struct that contains:
        - Position, size
        - Texture and shader IDs

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
