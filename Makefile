LDFLAGS=`pkg-config --libs sdl2`
CXXFLAGS=`pkg-config --cflags sdl2`

reflect-ray: reflect-ray.o
	c++ $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	-rm *.o
	-rm reflect-ray