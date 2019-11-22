viewer: build
	@./build/xodr_viewer/xodr_viewer

.PHONY: build
build:
	@mkdir -p build
	@cd build && cmake ../src
	@cd build && make -j `nproc --all`

clean:
	@rm -rf build

test: build
	@./build/xodr/xodr_tests
