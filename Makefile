EXTERNAL = external

raygui_URL_LIB = https://github.com/raysan5/raygui/archive/refs/tags/4.0.tar.gz
raylib_URL_LIB = https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_linux_amd64.tar.gz

$(EXTERNAL):
	@mkdir $@

install_raylib:$(EXTERNAL)
	@sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
	@cd ./external && mkdir raylib-5.0 && cd raylib-5.0 && curl -L $(raylib_URL_LIB) -o raylib-5.0.tar.gz && tar -xzvf raylib-5.0.tar.gz --strip-components=1 && rm raylib-5.0.tar.gz
# install_raygui:$(EXTERNAL) $(EXTERNAL)/raygui
# 	@cd ./external/raygui && curl -o raygui.h https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h
install_raygui:$(EXTERNAL)
	@cd ./external && mkdir raygui && cd raygui && curl -L $(raygui_URL_LIB) -o raygui.tar.gz && tar -xzvf raygui.tar.gz --strip-components=1 && rm raygui.tar.gz


build: 
	@mkdir -p build/
	@RAYLIB_PATH="./external/raylib-5.0/lib"; \
	gcc -I"./external/raylib-5.0/include" -L"./external/raylib-5.0/lib" -o ./build/main ./src/main.c -lraylib -lm -lglfw -ldl -lpthread -w

run:
	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
	./build/main 127.0.0.1 8080 
run_saber:
	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
	LIBGL_ALWAYS_SOFTWARE=1 ./build/main 127.0.0.1 8080 

clean : 
	rm -rf build

# EXTERNAL = external

# raylib_URL_LIB = https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_linux_amd64.tar.gz

# RAYLIB_PATH = ./external/raylib-5.0
# RAYGUI_PATH = ./external/raygui
# BUILD_DIR = build
# SRC_DIR = src
# TARGET = $(BUILD_DIR)/main
# SRCS = $(SRC_DIR)/main.c
# CFLAGS = -I"$(RAYLIB_PATH)/include" -I"$(RAYGUI_PATH)" -L"$(RAYLIB_PATH)/lib"  -lraylib -lm -lglfw -ldl -lpthread -w

# all: $(TARGET)

# $(TARGET): $(SRCS)
# 	@mkdir -p $(BUILD_DIR)
# 	gcc $(SRCS) $(CFLAGS) -o $(TARGET)

# clean:
# 	rm -rf $(BUILD_DIR)


# install_raylib:$(EXTERNAL)
# 	@sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
# 	@cd ./external && mkdir raylib-5.0 && cd raylib-5.0 && curl -L $(raylib_URL_LIB) -o raylib-5.0.tar.gz && tar -xzvf raylib-5.0.tar.gz --strip-components=1 && rm raylib-5.0.tar.gz
# install_raygui:$(EXTERNAL) $(EXTERNAL)/raygui
# 	@cd ./external/raygui && curl -o raygui.h https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h


# run:
# 	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
# 	./build/main
# run_saber:
# 	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
# 	LIBGL_ALWAYS_SOFTWARE=1 ./build/main
