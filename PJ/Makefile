.PHONY: clean build compile run run-one

clean:
	rm -rf build
	find test/ -type f ! -name '*.fmj' -exec rm -f {} +

build:
	@mkdir -p build && \
	cd build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release && \
	make --no-print-directory -j$$(nproc)

compile: build
	@cd test && \
	for file in *.fmj; do \
		base="$$(basename "$$file" .fmj)" && \
		../build/tools/main/main "$$file" || exit 1 && \
		arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static \
			-o "$$base" "$$base.s" ../vendor/libsysy/libsysy32.s -lm || exit 1; \
	done

run:
	cd test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "s" ]; then \
			echo "$${file%%.*}"; \
			qemu-arm $${file%%.*}; \
		fi; \
	done; \

run-one:
	@test -n "$(word 2, $(MAKECMDGOALS))" || { echo "Usage: make run-one filename.fmj"; exit 1; }
	@file="$(word 2, $(MAKECMDGOALS))" && \
	base=$$(basename "$$file" .fmj) && \
	cd test && \
	qemu-arm "$$base" || exit 1; \


