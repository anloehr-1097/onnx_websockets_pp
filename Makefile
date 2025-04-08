build:
	echo "Building project."
	cd build && cmake .

clean:
	echo "Cleaning build dir."
	rm -rf build

run_server: 
	echo "Running server."
	./build/main

run_client:
	echo "Running client."
	python main.py


test:
	echo "Testing."
