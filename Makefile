CC = c++ --std=c++11
CFLAGS = -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -L/usr/local/lib -lmongocxx -lbsoncxx
MAIN = test
OBJECTS = $(MAIN).o

$(MAIN): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

.cpp.o: $@.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o $(MAIN)