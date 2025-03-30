-include config.mk

reflect-ray: reflect-ray.o
	c++ $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	-rm *.o
	-rm reflect-ray