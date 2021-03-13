CC?=gcc

all:
	$(CC) -m64 -o xrinfo -Wall -Werror xrinfo.c -lopenxr_loader -lpthread -lXrandr -lSM -lICE -lX11 -lXext -lOpenGL -lGLX -lGLU -lSDL2 -lm

clean:
	rm xrinfo
