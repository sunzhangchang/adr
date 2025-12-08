CC = gcc
CFLAGS = -O2 -Wall

# 如果系统有 pkg-config，则使用它获取 SDL2/SDL2_ttf 的编译/链接参数
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 SDL2_ttf 2>/dev/null)

ifeq ($(SDL_CFLAGS),)
    # pkg-config 不可用，保留默认（Windows/MinGW）回退
    CFLAGS +=
    LDLIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
else
    CFLAGS += $(SDL_CFLAGS)
    LDLIBS = $(SDL_LIBS)
endif

SOURCES = main.c ui.c button.c scene.c inventory.c story.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = adr.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
