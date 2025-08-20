# Sentry for GameMaker

GameMaker extension for error monitoring and crash reporting with [Sentry](https://sentry.io/).

## Features

- **Automatic Error Reporting**: Captures unhandled exceptions and sends them to Sentry
- **Crash Reporting**: Native crash reporting using Sentry's crashpad handler
- **Custom Messages**: Send custom messages and breadcrumbs to Sentry
- **Performance Monitoring**: TODO - Track performance metrics (planned)
- **Release Tracking**: TODO - Associate errors with specific game releases (planned)

## Minimum Supported GameMaker Versions

| GameMaker Version | Supported |
|------------------|-----------|
| 2023.11+         | ✅        |
| 2023.8           | ✅        |
| Earlier versions | ❓ (untested) |

## Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| macOS    | ARM64       | ✅     |
| macOS    | x86_64      | TODO   |
| Windows  | x86_64      | TODO   |
| Linux    | x86_64      | TODO   |

## Installation

1. Download the latest `.yymps` package from the [Releases](TODO_RELEASES_URL) page
2. In GameMaker, drag the `.yymps` file into your project window
   - Or go to **Tools → Import Local Package** and select the file
3. In the import dialog:
   - Select the "extensions" folder
   - Click "Add"
   - Choose "Import"

## Configuration

1. In the Asset Browser, expand **Extensions** and double-click **Sentry**
2. In the Extension Properties, configure your Sentry settings:
   - Set **DSN** to your Sentry project DSN
   - Set **sample_rate** to control error sampling (default: `1.0` = 100%, `0.5` = 50%, etc.)
   - Set **debug** to `true` for development, `false` for production  
   - Set **setup_exception_handler** to `true` to automatically capture unhandled exceptions

TODO_SCREENSHOT_EXTENSION_PROPERTIES

That's it! The extension will automatically initialize when your game starts if a DSN is configured.

## Quick Start

### Automatic Setup (Recommended)

The extension automatically initializes Sentry when your game starts - no additional code required! Just configure your DSN in the extension properties and you're ready to go.

### Manual Initialization

For more control over initialization, you can disable automatic setup and initialize manually:

```gml
// Initialize Sentry
var dsn = "YOUR_SENTRY_DSN_HERE";
var db_path = game_save_id + ".sentry-native";
var sample_rate = 1.0; // 1.0 = 100% sampling, 0.5 = 50% sampling
var debug_enabled = 1.0; // 1.0 for debug, 0.0 for production

sentry_init(dsn, db_path, sample_rate, debug_enabled);

// Set up exception handler
exception_unhandled_handler(sentry_exception_handler);
```

### Capturing Custom Messages

```gml
// Capture an error message
sentry_capture_message(3, "Something went wrong!");

// Capture with different severity levels:
// 0 = debug, 1 = info, 2 = warning, 3 = error, 4 = fatal
sentry_capture_message(1, "Player reached level 10");
```

### Capturing Exceptions

```gml
try {
    // Code that might throw an exception
    var result = risky_operation();
} catch (exception) {
    // Convert exception to JSON and send to Sentry
    var exception_json = json_stringify(exception);
    sentry_capture_exception_json(exception_json);
}
```

## Configuration Options

### Sample Rate

The `sample_rate` option controls what percentage of errors are sent to Sentry. This is useful for high-traffic applications where you want to reduce the volume of events while still getting a representative sample.

- **Range**: `0.0` to `1.0`
- **Default**: `1.0` (100% of errors are sent)
- **Examples**:
  - `1.0` = Send 100% of errors (recommended for development)
  - `0.5` = Send 50% of errors
  - `0.1` = Send 10% of errors (good for high-volume production games)
  - `0.0` = Send 0% of errors (effectively disables error reporting)

The sampling is deterministic per event, meaning the same error will consistently be either included or excluded based on the sample rate.

## API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `sentry_init(dsn, db_path, sample_rate, debug)` | Initialize Sentry with DSN, database path, sampling rate, and debug mode |
| `sentry_close()` | Clean shutdown of Sentry |
| `sentry_auto_init()` | Automatic initialization using extension options |

### Message Capture

| Function | Description |
|----------|-------------|
| `sentry_capture_message(level, message)` | Send a message to Sentry with severity level |
| `sentry_capture_exception_json(json_string)` | Send exception data as JSON |

### Exception Handling

| Function | Description |
|----------|-------------|
| `sentry_exception_handler(exception_struct)` | Default exception handler for GameMaker |

## Data Storage and Logging

The extension stores data and logs in the following locations:

### Sentry Database
- **Location**: `{game_save_id}.sentry-native/` directory
- **Purpose**: Local cache for events before they're sent to Sentry
- **Note**: This directory is created automatically in your game's save directory

### Debug Logs (Debug Mode Only)
When debug mode is enabled, detailed logs are written to:
- **Extension logs**: `/tmp/sentry_gm_debug.log`
- **Sentry Native logs**: `/tmp/sentry_native_debug.log`

These logs contain detailed information about Sentry initialization, event capture, and any errors that occur.

## Troubleshooting

### Common Issues

**Extension not loading:**
- Ensure native libraries are correctly copied to the extension folder
- Check that your GameMaker version supports the required features

**Messages not appearing in Sentry:**
- Verify your DSN is correct
- Check debug output in GameMaker's console
- Ensure your Sentry project is configured to accept messages
- Enable debug mode and check log files at `/tmp/sentry_gm_debug.log`

**Crashes not being reported:**
- Verify `crashpad_handler` binary is present and executable
- Check that `sentry_close()` is called before game exit when possible
- Check debug logs for crashpad handler errors

### Debug Information

Enable debug mode in the extension properties to get detailed logging information. Debug logs will help you understand:
- Whether Sentry is initializing correctly
- If events are being captured and sent
- Any errors in the crashpad handler setup
- Network connectivity issues

## Building from Source

See [CONTRIBUTING.md](TODO_CONTRIBUTING_URL) for instructions on building the extension from source.

## Contributing

We welcome contributions! Please see our [Contributing Guide](TODO_CONTRIBUTING_URL) for details on:

- Setting up the development environment
- Building and testing changes
- Submitting pull requests

Join our community:
- [GitHub Discussions](TODO_DISCUSSIONS_URL)
- [GitHub Issues](TODO_ISSUES_URL)

## Resources

- [Sentry Documentation](https://docs.sentry.io/)
- [GameMaker Extension Documentation](https://manual.gamemaker.io/monthly/en/The_Asset_Editors/Extensions.htm)
- [Sentry for Other Game Engines](https://docs.sentry.io/platforms/)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The extension includes the Sentry Native SDK, which is also licensed under the MIT License.