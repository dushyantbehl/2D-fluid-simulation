CXX=g++
EXE=run
FILES= main.cpp RenderTexture.cpp debug.cpp
COMPILE=-c
LINK=-o
OBJECTS= obj/main.o obj/RenderTexture.o obj/debug.o
OBJDIR= obj/
LIBS= -lm -lGL -lGLU -lglut -lGLEW -lCg -lCgGL
DISABLE_WARNING=-w
NVIDIA=optirun

#Compile all the files
all:
	$(CXX) $(COMPILE) $(FILES) $(LIBS)
	mv *.o $(OBJDIR)
	$(CXX) $(LINK) $(EXE) $(OBJECTS) $(LIBS) 
#Compile and execute the code (using optirun)
exec:
	$(CXX) $(COMPILE) $(FILES) $(LIBS)
	mv *.o $(OBJDIR)
	$(CXX) $(LINK) $(EXE) $(OBJECTS) $(LIBS)
	$(NVIDIA) ./$(EXE)
#To run the program without optirun
run:
	./$(EXE)
clean:
	rm -rf $(EXE)
	rm -rf $(OBJECTS)
	rm -rf *.h~ *.cpp~ *Makefile~
