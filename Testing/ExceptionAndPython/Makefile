all: main foo.so
	

main:
	g++ -g -Wall -Wunused-variable  foo_main.cxx -o main

foo.so:
	g++ -c -g -Wall -Wunused-variable  foo.cxx -o foo.o
	swig -python -c++  -o foo_wrap.cxx foo.i
	g++ -c -g "-I`python -c "import sys; print sys.exec_prefix"`/include/python`python -c "import sys; print sys.version[:3]"`" foo_wrap.cxx -o foo_wrap.o
	g++ -shared foo_wrap.o foo.o -o _foo.so -g
clean:
	rm -f *.o *.so foo.py foo.pyc foo_wrap.cxx main
