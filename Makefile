# make
all:main.cpp
	c++ -I /usr/local/boost_1_55_0 main.cpp -o main
clean:
	rm -f main