# 3350 lab1
# to compile your project (Windows), type make and press enter
# to compile your project (Mac), type 'make mac' and press enter


all: lab1Windows

mac: lab1Mac

lab1Windows: lab1.cpp 
	g++ lab1.cpp libggfonts.a -Wall -olab1 -lX11 -lGL -lGLU -lm	

lab1Mac: lab1.cpp	
	g++ lab1.cpp -Wall -olab1 -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -framework OpenGl -framework Cocoa -lm -lGL

clean:
	rm -f lab1

