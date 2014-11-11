
# Define the constants for folders and files.
PATH_CUR            := $(shell pwd)
PATH_SRC            := $(PATH_CUR)/src/
PATH_INC            := $(PATH_CUR)/include/
PATH_OBJ            := $(PATH_CUR)/obj/
PATH_OUT_DBG        := $(PATH_CUR)/debug/
PATH_OUT_REL        := $(PATH_CUR)/release/
PATH_LIB_3PARTY 	:= $(PATH_CUR)/3-party-lib/
PATH_LIB_UTHASH  	:= $(PATH_LIB_3PARTY)uthash/
NAME_OBJ            := cluster
NAME_ENTRY 			:= entry
NAME_LIB            := lib$(NAME_OBJ)
ifeq ($(DEBUG), yes)
	PATH_OUT := $(PATH_OUT_DBG)
else
	PATH_OUT := $(PATH_OUT_REL)
endif


# Create the relevant folders to store objective and executable files.
FOLDER_OBJ          := $(shell mkdir -p $(PATH_OBJ))
FOLDER_OUT_DBG      := $(shell mkdir -p $(PATH_OUT_DBG))
FOLDER_OUT_REL      := $(shell mkdir -p $(PATH_OUT_REL))


# Specify the compilation options.
CC                  := gcc
FLAG                := -fPIC
LIB 				:= -lfuzzy
ARCH                := ar
ARCH_OPT            := rcs
ifeq ($(DEBUG), yes)
	FLAG := $(FLAG) -g
endif
ifeq ($(COVERAGE), yes)
	FLAG := $(FLAG) -O0 --coverage
endif


# List the dependencies for project building.
DEPENDENCY          := ds spew group pattern cluster
VPATH               := $(PATH_INC)


# List the project building rules.
build_executable: $(DEPENDENCY) build_entry
	$(CC) $(FLAG) -I$(PATH_INC) $(PATH_OBJ)*.o $(LIB) -o $(PATH_OUT)$(NAME_ENTRY)

build_static_lib: FLAG := $(FLAG) -fPIC
build_static_lib: $(DEPENDENCY)
	$(ARCH) $(ARCH_OPT) $(PATH_OUT)$(NAME_LIB).a $(PATH_OBJ)*.o

build_entry:
	$(CC) $(FLAG) -I$(PATH_INC) -c $(PATH_SRC)$(NAME_ENTRY).c -o $(PATH_OBJ)$(NAME_ENTRY).o

$(DEPENDENCY):
	$(CC) $(FLAG) -I$(PATH_INC) -I$(PATH_LIB_UTHASH) -c $(PATH_SRC)$@.c $(LIB) -o $(PATH_OBJ)$@.o


# List the project cleaning rule.
.PHONY: clean
clean:
	rm -rf $(PATH_OBJ) $(PATH_OUT_DBG) $(PATH_OUT_REL)

