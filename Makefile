sockets: main.cc
	$(CXX) $(shell pkg-config --libs --cflags libsystemd) $< -o $@
