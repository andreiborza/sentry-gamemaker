.PHONY: all build extension clean help

# Default target
all: build

# Build the GameMaker extension
build: extension

# Build the extension (main target)
extension:
	@echo "Building Sentry GameMaker Extension..."
	@mkdir -p build
	@cd build && cmake ..
	@cd build && make
	@echo "✅ Extension built successfully!"
	@echo "📁 Library: demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib"
	@echo "📁 Handler: demo/SentryGameMaker/extensions/Sentry/crashpad_handler"

# Clean build files and extension outputs  
clean:
	@echo "Cleaning build files..."
	@rm -rf build
	@rm -f demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib
	@rm -f demo/SentryGameMaker/extensions/Sentry/crashpad_handler
	@echo "✅ Clean complete!"

# Help target
help:
	@echo "Available targets:"
	@echo "  build      - Build the GameMaker extension (default)"
	@echo "  extension  - Build the GameMaker extension"
	@echo "  clean      - Clean build files and extension outputs"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Output files:"
	@echo "  demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib  - GameMaker extension library"
	@echo "  demo/SentryGameMaker/extensions/Sentry/crashpad_handler    - Crash handler binary"