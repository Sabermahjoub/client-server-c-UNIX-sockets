EXTERNAL = external

raylib_URL_LIB = https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_linux_amd64.tar.gz

$(TEST)/bin/%: $(TEST)/%.c
	@gcc $< -o $@ -I"./external/criterion-v2.3.3/include/" -L"./external/criterion-v2.3.3/lib/" -lcriterion

$(TEST)/bin:
	@mkdir $@

$(TEST)/images:
	@mkdir $@

$(EXTERNAL):
	@mkdir $@
$(EXTERNAL)/raygui:
	@mkdir $@


install_raylib:$(EXTERNAL)
	@sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
	@cd ./external && mkdir raylib-5.0 && cd raylib-5.0 && curl -L $(raylib_URL_LIB) -o raylib-5.0.tar.gz && tar -xzvf raylib-5.0.tar.gz --strip-components=1 && rm raylib-5.0.tar.gz
install_raygui:$(EXTERNAL) $(EXTERNAL)/raygui
	@cd ./external/raygui && curl -o raygui.h https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h

build: 
	@mkdir -p build/
	@RAYLIB_PATH="./external/raylib-5.0/lib"; \
	gcc -I"./external/raylib-5.0/include" -L"./external/raylib-5.0/lib" -o ./build/main ./src/main.c -lraylib -lm -lglfw -ldl -lpthread -w

run:
	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
	./build/main
run_saber:
	@export LD_LIBRARY_PATH=./external/raylib-5.0/lib; \
	LIBGL_ALWAYS_SOFTWARE=1 ./build/main
