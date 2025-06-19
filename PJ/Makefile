
clean: 
	$(RM) test/*.my \

run:
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "s" ]; then \
			echo "$${file%%.*}"; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o "$${file%%.*}" "$${file%%.*}.s" ../vendor/libsysy/libsysy32.s -lm; \
			qemu-arm $${file%%.*}; \
		fi; \
	done; \

