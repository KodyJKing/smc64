- Code quality:
    - Move all global state into a struct for cleaner reinitilization on halo1.dll reload.
    - [Blocker] Fix file naming inconsistencies (mostly casing).
    - Break up Entity type by type. Eg: Vehicle, Biped, Weapon, etc.
    - Create a fork of LibSM64 that exposes bone transforms.
    - [Major] Fix namespace naming inconsistencies in Mario mod. (should match DevTools mod)
    - [Major] Rename Entity to Object to agree with Halo's naming conventions.
        - Source
        - Ghidra
        - Cheat Engine
        - Documentation

- Tooling
    - Track tags and assets in git.
        - Store diffs relative to vanilla tag values.
        - Export tag data to XML for human readable diffs.
    - [Major] Save manager for better testing inner loop.
    - [Major] Unit testing framework for halo1.dll untility functions to verify understanding.

- Bugs and Stability:
    - Audit all raw memory accesses. Replace with safe wrappers in non-performance critical code.
    - [Blocker] Fix Mario being able to glancing-wall-kick out of shell riding.
    - [Major] Create a user facing recovery system for when Mario gets stuck.
    - [Minor] Fix Mario visually sinking into fast elevators (seems to be caused by a 1-frame delay in the object transforms).
    - [Minor] Fix Mario spazzing out near problem triangles in Covenant hallways.
    - [Minor] Fix Mario's lighting not updating when he is in a vehicle. (caused by model's velocity not updating, I think)

- Performance:
    - [Major] Optimize BSP loading transitions.

- Testing:
    - [Blocker] Do a full playthrough on the release build.
    - [Blocker] Test every map for problem triangles and delete them.
        - Pay special attention to Covenant hallways and columns.

- Gameplay:
    - Movement:
        - Fix wall kicks for near-vertical walls.
            - Fork LibSM64 to make wall detection more forgiving.
            - OR hack: Place a vertical wall between Mario and a wall when he is airborne and close.
        - Rocket/grenade jumps should trigger flight when cap is active.
        - Collision:
            - Entity collision:
                - Support collision with bipeds.
                    - Allow bi-directional pushing. (might need custom collision/solving rather than using Mario engine)
                - Support per tag type model override for finer developer control.
            - Level collision:
                - Level tweak tooling.
                    - Allow placement of planes, boxes, capsules.
                    - Allow deletion of unwanted faces from Halo's BSP.
                    - Level tweak exporter.
    - Combat:
        - Melee
            x Implement melee and shell damage interactions.
            - Implement less naive rules for deciding when a hand/foot hitbox is live. Should be based off of Mario's animation state OR velocity.
            - Try out relative speed scaled melee damage.
    - Items:
        - Coins, placed on level and dropped for certain kills.
            - Add coin counter to HUD.
            - Experiment with coin uses:
                - Health recovery?
                - Consume for special moves/abilities?
                    - Crazy box jump?
        - Ridable jackal shield. Dropped when ground pounding a jackal. Rideable as koopa shell.
        - Add hard to reach power ups near enemy encounters. Reward players for agressive movement.
        - Add optional obstacle sections to levels with 
        - Add Mario themed weapons:
            - Bobomb cannon.
            - Tennis racket. (melee and projectile deflection)
            - Golf club. (precision long range grenade launching)
    - Side levels:
        - Add paintings to access side levels.


- Presentation:
    - Animation:
        - Inverse kinematics:
            - Arm IK:
                - [Major] Smooth out arm IK transition from busy to not busy.
                - Implement pole targets so Mario's elbows bend naturally.
            - Torso IK:
                - Try out making Mario lean slightly as he aims up and down.
        - Head:
            - Bias Mario's look direction towards the direction the camera is facing.
            - Lean Mario's head sideways towards weapon when he is firing/ADSing.
        - Drive Mario pose from Chief's 
            x in cutscenes.
            x and vehicles.
            - When throwing grenade (left arm only).
            - When reloading (arms only).
            - Fix Mario's hands and feet being enlarged when in vehicles (due to IK setting end effector transform.w, I believe).
    
    - Third person camera:
        - Render a false projectile exiting the player's barrel.
            - False projectile should converge with the actual projectile over distance.
        - [Critical] Move or disable all FPV weapon lights/effects.
    
    - Sound:
        - [Blocker] Scale Mario sound by game's volume setting.

- Usability:
    - Keybinds:
        - [Critical] Implement remappable keybinds.
        - [Critical] Reverse MCC's keybinds so we can hook into those too.
        - [Critical] Address some keys being detected when window is not focused.

