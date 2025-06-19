
# 主Makefile (位于项目根目录)
.PHONY: build compile run clean

# 构建编译器
build:
	@mkdir -p build && \
	cd build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release && \
	make --no-print-directory -j18

# 编译test目录下所有.fmj文件
compile: build
	cd test && \
	for file in ./*.fmj; do \
		../build/tools/main/main "$$file" \
		arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o "$${file%%.*}.out" "$${file%%.*}.s" ../vendor/libsysy/libsysy32.s -lm; \
	done

# 运行测试程序
run: compile
	@cd build && \
	for file in ../test/*.s; do \
		qemu-arm -B 0x1000 "$${file%.*}" || exit 1; \
	done

# 清理
clean:
	rm -rf build


clean:
	rm -rf build test/*.my test/*.s test/*.out \

# compile:
# 	cd $(CURDIR)/test && \
# 	for file in $$(ls .); do \
# 		if [ "$${file#*.}" = "s" ]; then \
# 			echo "$${file%%.*}"; \

# 			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o "$${file%%.*}" "$${file%%.*}.s" ../vendor/libsysy/libsysy32.s -lm; \
# 			qemu-arm $${file%%.*}; \
# 		fi; \
# 	done; \

run:
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "s" ]; then \
			echo "$${file%%.*}"; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o "$${file%%.*}" "$${file%%.*}.s" ../vendor/libsysy/libsysy32.s -lm; \
			qemu-arm $${file%%.*}; \
		fi; \
	done; \

