LIBS=-lglut -lGLU -lGLEW
CCFLAGS=-g

all: Display

Display: Display.o shader.o FrameBuffer.o
	g++ Display.o shader.o FrameBuffer.o $(LIBS) $(CCFLAGS) -o $@

%.o: %.cpp
	g++ $< $(LIBS) $(CCFLAGS) -c -o $@

clean:
	rm *.o Display
