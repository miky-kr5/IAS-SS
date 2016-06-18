CXX = g++
TARGET = ias-ss
OBJECTS = ias_ss.o robot.o
CFLAGS = -ansi -pedantic -Wall -g `pkg-config --cflags playerc++` -g
LDLIBS = `pkg-config --libs playerc++` -lboost_system -lpthread

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

-include $(OBJECTS:.o=.d)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CFLAGS) $*.cpp > $*.d

clean:
	$(RM) $(TARGET) $(OBJECTS) *.d
