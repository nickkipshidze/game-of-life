CXX = clang++
CXXFLAGS = -O3 -Wall -Wextra -std=c++23 -I./include/
LDLIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

TARGET = main
SRC = $(wildcard ./src/*.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o ./build/$(TARGET) $(LDLIBS)

run: $(TARGET)
	./build/$(TARGET)

dist:
	mkdir -p ./build/lib
	cp -r ./assets ./build/assets

	$(CXX) $(CXXFLAGS) $(SRC) -o ./build/Life $(LDLIBS) -Wl,-rpath,'$$ORIGIN/lib'

	@for lib in $$(ldd ./build/Life | awk '/=>/ {print $$3}' | grep '^/'); do \
		cp -L "$$lib" ./build/lib/; \
	done

clean:
	rm ./build/*

.PHONY: run clean
