- Disable fall death timer when Mario is in control.

- Fix spurious collisions with large distant triangles.
    - Create better tooling to investigate these triangles. Determine if they are actual collidable geometry in game.
    - Experiment with dynamic collision mesh gen for static level geometry.

- Keep mario model's lighting up to date.
    - Call light update function on mario's display object every frame.
    - See (halo1.dll+B49721 : Write model light)

- Fix Mario's shadow clipping.
    -Try updating his tag's bounds.

- Render a false projectile exiting the player's barel.
    - False projectile should converge with the actual projectile over distance.
    
- Fix wall kicks for near-vertical walls.
    - Place a vertical wall between Mario and a wall when he is airborne and close.

- Blend Mario's animation with Chief's.
