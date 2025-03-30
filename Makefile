-include config.mk

reflect-ray: reflect-ray.o
	$(CXX) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	-rm *.o
	-rm reflect-ray