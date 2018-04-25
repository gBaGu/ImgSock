BINDIR := bin

all: mkdirs
	g++ -std=c++14 src/*.cpp -o $(BINDIR)/run \
	-lstdc++fs -lpthread -lboost_system \
	`pkg-config opencv --cflags --libs`

mkdirs:
	mkdir -p $(BINDIR)
