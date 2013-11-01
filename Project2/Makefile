# Basic idea:
# - If you only need to build the test file, make test
# - Building everything will build the test, all of the server related code, and rebuilt the kernel.
#   - cd to pm dir, make
#   - cd to tools dir, make hdboot

# Directories

PROJECT_DIR = /mnt/Shared/Project2
PM_DIR = /usr/src/servers/pm
TOOLS_DIR = /usr/src/releasetools

# Building

all: clean pm tools test

pm:
	./copyAllFiles
	cd $(PM_DIR); make

tools:
	cd $(TOOLS_DIR); make hdboot

test:
	./copyAllFiles
	gcc -o test test.c

clean:
	rm -f test
