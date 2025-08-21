# Code Structure

This repository contains a GameMaker extension for Sentry error monitoring and crash reporting.

## Directory Structure

```
├── src/                          # Extension development source
│   ├── extensions/Sentry/        # GameMaker extension files
│   │   ├── Sentry.yy            # Extension definition
│   │   └── sentry.gml           # GML wrapper functions
│   ├── sentry-native/           # Sentry native SDK
│   │   ├── include/sentry.h     # C headers
│   │   └── lib/                 # Native libraries
│   └── sentry_gm.cpp            # C++ bridge code
├── demo/SentryGameMaker/        # Demo GameMaker project
├── build/                       # Build output
├── CMakeLists.txt              # CMake build configuration
└── Makefile                    # Build automation
```

## Key Components

- **Extension Source** (`src/`): The main extension development happens here
- **Demo Project** (`demo/SentryGameMaker/`): Showcases the extension in action
- **Build System**: Uses CMake and Makefile for compiling the native components
- **Native Integration**: Built on top of sentry-native SDK for crash reporting

The extension provides GML functions that bridge to the underlying sentry-native C library through C++ wrapper code.