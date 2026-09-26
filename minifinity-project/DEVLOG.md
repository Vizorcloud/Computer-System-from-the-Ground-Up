## Code Structure
- game.c: co-ordinates the state of the game. Interacts with the keyboard and graphics modules
- render.c: draws the game state to the screen
- keyboard.c: takes input from two keyboards

## Workflow
Adding our own mango modules: 
- paste the source file into the `mylib` directory
- in `mylib/Makefile`, add the name of the source file after `LIBMANGO_MODULE_SOURCES`
- rebuild `libmymango.a` (in `mylib`, run `make clean` then `make`). This should automatically put `libmymango.a` in the outer project directory
- returning to the project directory, run `make run` to run `main.c`

## Integration
Sunday, Mar 14
- rewrote word generation code

Saturday, Mar 13
- both keyboards working simultaneously, reading characters working
- TODO: when wrong letters come in, shade them the right color, minimal redraw
- TODO: get random word generation to work correctly (split function?)

## Graphics
Thursday, Mar 12
- given prompts for both players, renders them onto the screen
- draws backgrounds for both players
- wrap text within textbox
- TODO: implement typing error, cursor

## POSSIBLE EXTENSIONS
- do the monkeytype thing where if you miss a letter but continue typing correctly it will jump to the right spot
