SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Alter these 4 variables according to your need
CPP_FILES = \
	clear_spatial_partition \
	clear_static_surfaces \
	add_surface \
	read_surface_data \
	surface_has_force \
	surf_has_no_cam_collision \
	load_static_surfaces \
	load_environmental_regions \
	load_area_terrain \
	clear_dynamic_surfaces \
	transform_object_vertices \
	load_object_surfaces

INCLUDE_PATH = sm64-api/sm64
LIBRARY_PATH = sm64-api/ld

SRC_FILES = $(patsubst %, $(SRC_DIR)/%.c, $(CPP_FILES))
OBJ_FILES = $(patsubst %, $(OBJ_DIR)/%.o, $(CPP_FILES))
BIN_FILES = $(patsubst %, $(BIN_DIR)/%.bin, $(CPP_FILES))

CC = mips-n64-gcc
AR = mips-n64-ar
LD = mips-n64-ld
CFLAGS = -Wall -Wextra -Wno-trigraphs -Wno-missing-braces -Wno-overflow \
	-Os -ffast-math -ftrapping-math -fno-associative-math -G0 \
	-I $(INCLUDE_PATH) -I $(INCLUDE_PATH)/libc \
	-mno-shared -march=vr4300 -mfix4300 -mabi=32 -mhard-float -mdivide-breaks \
	-fno-stack-protector -fno-common -mno-abicalls -fno-strict-aliasing -ffreestanding -fwrapv \
	-DTARGET_N64 -D_LANGUAGE_C -DVERSION_US=1 -D_FINALROM=1 -DNDEBUG=1 \
	-nostdinc
PYTHON = python3

all: $(BIN_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ_DIR)/%.ld:
	$(PYTHON) ./patch_ldscript.py $@

$(BIN_DIR)/%.bin: $(OBJ_DIR)/%.o $(OBJ_DIR)/%.ld $(BIN_DIR)
	$(LD) -o $@ -L. -L $(LIBRARY_PATH) --oformat binary -T $(OBJ_DIR)/$*.ld -M $< > /dev/null

$(OBJ_DIR):
	mkdir -pv $(OBJ_DIR)

$(BIN_DIR):
	mkdir -pv $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
.SECONDARY: