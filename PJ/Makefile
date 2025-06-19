.PHONY: build compile run clean

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

run: compile
	cd test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "s" ]; then \
			echo "$${file%%.*}"; \
			qemu-arm $${file%%.*}; \
		fi; \
	done; \