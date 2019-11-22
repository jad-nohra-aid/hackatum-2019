viewer: build
	@$(CURDIR)/build/xodr_viewer/xodr_viewer

.PHONY: build
build:
	@mkdir -p build
	@cd build && cmake ../src
	@cd build && make -j `nproc --all`

clean:
	@rm -rf build

test: build
	@cd $(CURDIR)/src/xodr && $(CURDIR)/build/xodr/xodr_tests
