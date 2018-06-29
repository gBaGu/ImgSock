BINDIR := bin

server: mkdirs
	g++ -std=c++14 src/server.cpp src/ImageIO.cpp src/ImageConverter.cpp src/ImageProcessor.cpp src/ImageProcessingUnit.cpp src/other.cpp \
	-o $(BINDIR)/run \
	-lstdc++fs -lpthread -lboost_system \
	`pkg-config opencv --cflags --libs` \
	/home/haik/neuron/darknet/darknet.so -DOPENCV=1 \

client: mkdirs
	g++ -std=c++14 src/client.cpp -o $(BINDIR)/client \
	-lboost_system \
	`pkg-config opencv --cflags --libs`

mkdirs:
	mkdir -p $(BINDIR)
