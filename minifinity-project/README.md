## Project Title

MangoType

## Team members

Zara Song & Maxsem Garcia 

## Project description | Member contribution

MangoType is a reimagining of the popular typing website "Monkeytype" into a local, multiplayer, versus experience where two players race against eachother to finish typing a completed prompt

Overview of Features:
    - Basic monkeytype functionality: random prompt generation, keyboard / user input validation, graphical interface, live results feedback
    - Additional game loop segments: a main menu for customization, a countdown, and a dedicated "stats" page at the very end
    - Multiplayer: connecting two keyboards through GPIO and co-ordinating interrupts, split screen
    - "Powerups": the visual effects to sabotage opponent typing speed further, made as simply and diabolical as possible (e.g. flipping letters upside down, making them barely visible, making them rainbow, etc)
    - A gliding cursor:
    - Live WPM:

Breakdown of Features:

Number. = Highlighted Functionalities
Letter. = Supporting Functionalities
----------------------------
1. Main Menu | **Zara**
    The menu renders a split-screen layout where each player has their own name input field on their side of the screen. Three shared settings — Timer, Powerups, and # Words — sit centered across the full display. Tab cycles focus between the 5 fields (P1_NAME → P2_NAME → TIME → POWERUPS → PROMPT). Both keyboards can navigate independently. Enter validates input: it checks that both names are non-empty and that word count ≥ powerup count, showing a red error message if not. Once valid, set.ready is set and the game proceeds.

    Zara: branch render provides accurate snapshots of my work. As suggested by the name, I was primarily responsible for designing and implementing a functionally adaptable, but visually minimalist, graphical interface. I also really wanted to practice good code architecture (e.g. modularity and intuitive abstractions, also goal 2); and since render.c itself was a higher-level module built on the graphics module gl, it lent well to my taking over the high-level design, particularly in the beginning (it became much more difficult to co-ordinate together over the design choices later, but that was more just a good lesson learned about deciding on module interfaces early and before their respective implementations. The headers and file structure of render is therefore a better showcase of my "architecting" than master ended up being).

    a. Tab-based focus cycling between 5 fields across both keyboards
    b. Per-field input routing (names accept any char, numeric fields only accept digits)
    c. Backspace support in all fields
    d. Enter key validation (non-empty names, words ≥ powerups)
    e. Red error message display on failed validation
    f. Gold highlight on active field, white on inactive
    g. Split panel layout with P1/P2 name fields on their respective sides
    h. Three shared settings fields centered across the full screen
    i. Defined the three-state game flow architecture (menu → countdown → game loop → results) and implemented the transitions between each segment
    j. Handled all git merge conflicts when integrating the render and maxsem branches, coordinating shared files and function interfaces
2. Graphics Rendering | **Zara (Primary) / Maxsem**
    The display is split vertically into two 960×1080 panels. Each frame is doublebuffered — every draw operation writes into both back-buffers and swaps, so there's never a visible tear. render_init computes textbox dimensions dynamically from word count, centering the text block vertically. The main render loop does surgical partial redraws, only the 2–3 characters around the cursor get redrawn per frame rather than the whole screen, keeping it fast. Full redraws are triggered only when powerups change.

    a. Vertical split-screen — two independent 960×1080 panels | **Zara**
    b. Double-buffered display to eliminate screen tearing | **Zara**
    c. Dynamic textbox sizing computed from word count at init | **Zara**
    d. Vertical centering of the text block within each panel | **Zara**
    e. Surgical partial redraws — only 2–3 chars around cursor per frame | **Maxsem**
    f. Full redraw triggered only on powerup activation/expiry | **Maxsem**
    g. Player name titles rendered above each text block | **Maxsem**
    h. Live HUD stats bar (accuracy, WPM, countdown) rendered each frame | **Maxsem**
    i. full-screen 3-2-1-GO! sequence on black before each game, each number centered and held for exactly 1 second | **Zara**
    j. bottom-of-screen notification banner that cycles through 5 colors (amber→yellow→green→cyan→magenta) every 100ms tick, only redraws when color changes to avoid flicker | **Maxsem**
    k. Message area explicitly cleared each frame when no message is active, preventing stale notifications from persisting | **Maxsem**
    l. Word-wrap layout engine shared across all rendering functions — walks the prompt string word by word, breaking to a new row when a word won't fit, keeping every render function in sync with the same layout logic | **Zara**
    m. Matryoshka-doll struct layering — frame contains textbox contains positioning/sizing data, an elementary HTML-like abstraction for player screens | **Zara**
3. Random Prompt generation | **Maxsem (Initial Design / word databank) / Zara (Improved Design with rand.h module)**
    Each player gets an independently generated random prompt via getprompt(nwords). Prompts are seeded with the current timer tick so they differ between runs. Both players get different word lists. The prompt string is built into a single flat char array with spaces between words, and an accuracy array of the same length is allocated and zeroed alongside it.

    a. Independent random prompt per player each game
    b. Timer-tick seeding so prompts differ between runs
    c. Flat char array layout with spaces between words
    d. Parallel accuracy array allocated and zeroed alongside prompt
    e. Bonus words rendered in dark green at game start
4. Statistics Tracking | **Maxsem**
    WPM is computed on correctly completed whole words only (standard 5-chars-per-word definition). Raw WPM counts every keystroke including errors. Accuracy is (raw_chars - error_chars) / raw_chars * 100. The HUD shows Accuracy, WPM, and countdown timer live during gameplay via render_stats. At game end, render_finish_stats snapshots and freezes these values, then computes the final score as wpm * accuracy + time_bonus + bonus_pts.

    a. WPM computed on correctly completed whole words only (5-char standard)
    b. Raw WPM counting every keystroke including errors
    c. Accuracy as (raw − errors) / raw × 100
    d. Live HUD display of all three stats during gameplay
    e. Per-player personal timer starting on first keypress
    f. End-game score formula: WPM × accuracy + time bonus + bonus points
    g. Stats snapshotted and frozen at finish time before animation delays
5. Input Handling | **Maxsem**
    Two completely independent PS2 keyboards are handled via interrupt-driven scancode queues with parity checking and timeout-based partial-frame recovery. During gameplay, game_update processes each character: backspace moves the cursor back and marks the character UNTYPED, spaces can only be typed as spaces, and the first keypress starts the player's personal timer. During the menu, kb_select routes numeric-only input to the number fields and free text to name fields.

    a. Two fully independent PS2 keyboard drivers
    b. Interrupt-driven scancode queues (no polling)
    c. Odd parity checking on every received byte
    d. Timeout-based partial frame recovery (500µs gap resets accumulator)
    e. Backspace moves cursor back and marks character UNTYPED
    f. Space characters can only be typed as spaces (no skipping)
    g. First keypress starts the player's personal stat timer
6. Hyper Performance Optimization (60 fps graphics with little to no input delay) | **Maxsem**
    The frame budget is 16,667 µs (~60fps). Renders are only triggered when input_dirty is set (immediate on keypress) or when the frame timer fires. The partial redraw system — erasing only the old cursor rect, redrawing only the 2–3 surrounding characters — keeps each frame extremely cheap. The animated cursor uses eased interpolation (step_cursor) rather than teleporting, and cursor-only animation runs on a separate path that skips all text redraws entirely.

    a. 16,667µs frame budget enforced by should_render
    b. Immediate render on keypress via input_dirty flag
    c. Cursor easing via step_cursor — smooth interpolation, instant on row change
    d. Cursor-only animation path that skips all text redraws
    e. Old cursor position erased by redrawing background rect, not clearing screen
    f. Both back-buffers kept in sync — every draw written to both buffers
7. Powerup System | **Maxsem**
    Bonus words are randomly selected at game start and rendered in dark green. When a player correctly completes a bonus word, a random powerup is applied to their opponent for 3 seconds. Three powerup types exist: FLIP (characters rendered 180° rotated using a pixel-copy scratch buffer), CAMO (text color set to nearly match the background — only 30 RGB units brighter), and RAINBOW (each character cycles through 7 colors based on a tick counter). Powerup expiry triggers a full redraw to restore normal rendering.

    a. Bonus words randomly selected at game start from the word pool
    b. Correct completion of a bonus word applies a powerup to the opponent
    c. FLIP powerup — characters rendered 180° rotated via pixel-copy scratch buffer
    d. CAMO powerup — text color set to only 30 RGB units above background
    e. RAINBOW powerup — each character cycles through 7 colors per tick
    f. All powerups last exactly 3 seconds then expire automatically
    g. Powerup activation triggers full redraw on the affected player's screen
    h. Notification messages shown to both players on powerup activation
8. End Game / Celebration Screen (render_finish_overlay, render_finish_stats, render_celebrate) | **Maxsem**
    When a player finishes or time runs out, their screen fades to near-black via a quadratic animation (20 steps, eased by s²). Stats are then revealed line by line with spin-loop pauses between each line for dramatic effect. The winner's panel enters a color-cycling celebration loop, cycling through 6 background/foreground color pairs every 300ms, showing WINNER! and the full scorecard until either player presses Tab to restart — which goto restart handles by jumping back to game_init and the menu.

    a. Quadratic fade-to-black animation on player finish (20 eased steps)
    b. Per-line stat reveal with spin-loop pauses for dramatic effect
    c. Score breakdown shown: accuracy multiplier, WPM, time bonus, bonus points, total
    d. Color-cycling winner celebration — 6 background/foreground pairs at 300ms intervals
    e. WINNER! and "Press TAB to restart" displayed on winner's panel
    f. Either player pressing Tab triggers full restart via goto restart
    g. Keyboard drain on restart to discard the Tab that triggered it
9.  Code Architecture & Tooling | **Zara**
    a. Module interface design — headers and file structure of render designed before implementation to enforce clean abstractions
    b. render.c architected as a high-level module cleanly layered on top of gl, mirroring how gl itself is layered on hardware
    c. Makefile pipeline: adding a new module triggers mylib rebuild → auto-packages libmymango.a to the outer directory → make run compiles and launches the full project in one step
    d. Smooth multi-branch workflow coordinating parallel development on render and maxsem branches with clean merge conflict resolution

## References
https://monkeytype.com/

## Self-evaluation

Our team believes that we executed the plan in our proposal very well. All of the major functionalities that we set out to achieve were functional, incredibely fast, and flashy in our demo. Moreover, we were able to implement many more functionalities above what we initially set out to do (such as the powerups and animations we implemented). A heroic moment we would like to share was pulling a full 12 hour all nighter the night before demo day to polish the performance of all the features in our project by overcoming the notorious speed barriers when it comes to rendering graphics on the mango pi. We both poured a ton of effort into the project and learned more about efficiently working in a team on a group codebase. Our process for working on seperate branches and then merging into a third, seperate one allowed both of us to work on our modular implementations without creating issues for the other, and eventually combine them. 

Maxsem: Zara was absolutely amazing at handling any conflicts that came with merging projects.

## Photos
Private YT link of a run of our project:
https://www.youtube.com/watch?v=O5pdKJMoLXI