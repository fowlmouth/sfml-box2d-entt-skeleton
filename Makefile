SRC := $(wildcard src/*.cpp src/*/*.cpp)
OBJ := $(SRC:.cpp=.o)

BIN := game

CXXFLAGS := $(shell pkg-config --cflags sfml-graphics) -Ientt/src -std=c++20
LDFLAGS := $(shell pkg-config --libs sfml-graphics) -lbox2d

ENTT_RELEASE := 3.12.2

ALL: $(BIN)
.PHONY: clean cleanall deps

%.o: %.cpp entt
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(BIN)

release: CXXFLAGS += -O3 -DRELEASE
release: $(BIN)

deps: entt

entt.tar.gz:
	curl --location --fail --silent --output entt.tar.gz https://github.com/skypjack/entt/archive/refs/tags/v$(ENTT_RELEASE).tar.gz
entt: entt.tar.gz
	mkdir entt
	tar xzvf entt.tar.gz --strip-components=1 -C entt

clean:
	rm -f $(OBJ) $(BIN)
cleanall: clean
	rm -rf entt.tar.gz entt
