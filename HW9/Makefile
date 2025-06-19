RM       = rm -rf
MAKEFLAGS = --no-print-directory

.PHONY: build clean rebuild handin

BUILD_DIR = $(CURDIR)/build

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release; \
	cd $(BUILD_DIR) && ninja
# cd $(BUILD_DIR) && ninja -v

clean: 
	@$(RM) build ; \
	$(RM) test/*5.* \

rebuild: clean build



.PHONY: run run-one gentests gentest-one patchdemo 

GENAST = $(CURDIR)/../HW5/build/tools/generatetests/gentests
GENIR = $(CURDIR)/../HW5/build/tools/main/main
GENQUAD = $(CURDIR)/../HW6/build/tools/main/main
GENSSA = $(CURDIR)/../HW7/build/tools/main/main
MAIN = $(BUILD_DIR)/tools/main/main
GENTESTS = $(BUILD_DIR)/tools/generatetests/gentests
PATCHDEMO = $(BUILD_DIR)/tools/patchdemo/patchdemo


run: $(MAIN)
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "4-prepared-xml.quad" ]; then \
			echo "Reading ${file}.4-prepared-xml.quad and ${file}.4-xml.clr"; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o "$${file%%.*}" "$${file%%.*}.s" ../vendor/libsysy/libsysy32.s -lm; \
			echo "Running the final assembly program........." ; \
			# qemu-arm -B 0x1000 $${file%%.*}; \
		fi; \
	done; \
	cd .. > /dev/null 2>&1 


# arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o