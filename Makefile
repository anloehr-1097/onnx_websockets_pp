build-project:
	echo "Building project."
	cd build && make

prep:
	echo "Prep cache."
	mkdir build && cd build && cmake ..
clean:
	echo "Cleaning build dir."
	rm -rf build

run_server: 
	echo "Running server."
	./build/main

run_client:
	echo "Running client."
	python main.py

all:
	# combination of all steps
	make clean && make prep && make build-project

test:
	echo "Testing."

run:
	./build/src/main

run-tests:
	./build/tests/cust_test
	cd py_client && python -m unittest
