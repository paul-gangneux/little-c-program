CFLAFS = -O2 -Wall -pedantic
DEBUG_FLAGS = -Wall -pedantic -ggdb
LIBS = `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf -lm -lGL -ldl

DIRS = obj/ obj/gameoflife/ obj/space/

OBJ_GOL := gol_run text_to_cells cell
OBJ_SPACE := space_run
OBJ := windowManager textLoader imageLoader eventHandler main

OBJ_GOL := $(addprefix gameoflife/, $(OBJ_GOL))
OBJ_SPACE := $(addprefix space/, $(OBJ_SPACE))
OBJ += $(OBJ_GOL) $(OBJ_SPACE)

OBJ := $(addprefix obj/, $(OBJ))
OBJ := $(addsuffix .o, $(OBJ))

program: $(OBJ)
	gcc $(CFLAGS) -I ./headers $^ $(LIBS) -o $@

obj/%.o: src/%.c headers/%.h | $(DIRS)
	gcc $(CFLAGS) -I ./headers -c $< -o $@

%/:
	mkdir $@

.PHONY: clean
clean: 
	rm -vr obj/ program
