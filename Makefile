# config

INCS = `pkg-config --cflags glfw3` \
	-I /Users/bugrayildiz/.bin/VulkanSDK/1.3.243.0/macOS/include

LIBS = `pkg-config --libs glfw3` \
	-L /Users/bugrayildiz/.bin/VulkanSDK/1.3.243.0/macOS/lib -lMoltenVK \
	-Wl,-rpath,/Users/bugrayildiz/.bin/VulkanSDK/1.3.243.0/macOS/lib

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
