arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra --static -o hw8test04 test/output_example/k9/hw8test04.s ./vendor/libsysy/libsysy32.s -lm
qemu-arm hw8test04