.PHONY: all build run clean

# Default target - show help
all: run

# Build the project
build:
	@mkdir -p build
	@cd build && cmake ..
	@cd build && make

# Run the sample
run: build
	@cd build && ./sample

# Clean build files
clean:
	@rm -rf build

# Help target
help:
	@echo "Available targets:"
	@echo "  build  - Build the project (default)"
	@echo "  run    - Build and run the sample"
	@echo "  clean  - Clean build files"
	@echo "  help   - Show this help message"