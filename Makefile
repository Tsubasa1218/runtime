
clean:
	rm -rdf build

prepare:
	make clean &&	mkdir build && cd build && cmake -DCMAKE_CXX_COMPILER=clang++ ../

buildit:
	cd build && cmake --build .

run:
	cd build && ./Form
  
