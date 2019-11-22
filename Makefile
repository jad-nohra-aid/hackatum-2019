viewer: build
	@./build/xodr_viewer/xodr_viewer

.PHONY: build
build:
	@mkdir -p build
	@cmake -B build src
	@cd build && make -j `nproc --all`

clean:
	@rm -rf build

test: build
	@./build/xodr/xodr_tests
