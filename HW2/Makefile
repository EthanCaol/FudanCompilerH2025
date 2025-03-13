MAKEFLAGS = --no-print-directory

.PHONY: build clean rebuild test handin

build:
	@cd tools/2.4b; make build; cd ../..
	@cd tools/2.5c; make build; cd ../..
	@cd tools/3.3d; make build; cd ../..
	@cd tools/3.12a; make build; cd ../..

clean:
	@cd tools/2.4b; make clean; cd ../..
	@cd tools/2.5c; make clean; cd ../..
	@cd tools/3.3d; make clean; cd ../..
	@cd tools/3.12a; make clean; cd ../..

rebuild: clean build

test:
	@cd tools/2.4b; make test; cd ../..
	@cd tools/2.5c; make test; cd ../..
	@cd tools/3.3d; make test; cd ../..
	@cd tools/3.12a; make test; cd ../..

handin:
	@if [ ! -f docs/report.pdf ]; then \
		echo "请先在docs文件夹下攥写报告, 并转换为'report.pdf'"; \
		exit 1; \
	fi; \
	filename="22300240008-曹奕伦"; \
	zip -q -r "docs/$$filename-hw2.zip" \
	  tools docs/report.pdf
