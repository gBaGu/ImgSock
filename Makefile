BINDIR := bin

all: mkdirs
	g++ -std=c++14 src/*.cpp -o $(BINDIR)/run \
	-lboost_system \
	`pkg-config opencv --cflags --libs`

mkdirs:
	mkdir -p $(BINDIR)
