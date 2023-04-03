# config

INCS = `pkg-config --cflags glfw3 vulkan`

LIBS = `pkg-config --libs glfw3 vulkan`

CC = cc -g3 $(LIBS) $(INCS)



# targets

.PHONY: all
all: bin/papirus-client;

.PHONY: deps
deps:
	brew install pkg-config
	brew install --HEAD glfw

.PHONY: clean
clean:
	rm -rf bin


# objects

bin/papirus-client: bin src/main.c
	$(CC) -o bin/papirus-client src/main.c

bin:
	mkdir -p bin
