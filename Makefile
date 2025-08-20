.PHONY: all build extension clean sync copy-extensions help

# Default target
all: build

# Build the GameMaker extension
build: extension

# Build the extension (main target)
extension: copy-extensions
	@echo "Building Sentry GameMaker Extension..."
	@mkdir -p build
	@cd build && cmake ..
	@cd build && make
	@echo "✅ Extension built successfully!"
	@echo "📁 Library: demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib"
	@echo "📁 Handler: demo/SentryGameMaker/extensions/Sentry/crashpad_handler"

# Copy extension files from src to demo (always runs)
copy-extensions:
	@echo "Copying Sentry extension files to demo project..."
	@mkdir -p demo/SentryGameMaker/extensions/Sentry
	@cp -r src/extensions/Sentry/* demo/SentryGameMaker/extensions/Sentry/
	@echo "✅ Extension files copied successfully!"

# Clean build files and extension outputs  
clean:
	@echo "Cleaning build files..."
	@rm -rf build
	@rm -f demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib
	@rm -f demo/SentryGameMaker/extensions/Sentry/crashpad_handler
	@echo "✅ Clean complete!"

# Sync changes from demo back to src
sync:
	@./scripts/sync-from-demo.sh

# Help target
help:
	@echo "Available targets:"
	@echo "  build           - Build the GameMaker extension (default)"
	@echo "  extension       - Build the GameMaker extension"
	@echo "  copy-extensions - Copy extension files from src to demo (without building)"
	@echo "  clean           - Clean build files and extension outputs"
	@echo "  sync            - Sync changes from demo back to src folder"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Output files:"
	@echo "  demo/SentryGameMaker/extensions/Sentry/libsentry_gm.dylib  - GameMaker extension library"
	@echo "  demo/SentryGameMaker/extensions/Sentry/crashpad_handler    - Crash handler binary"