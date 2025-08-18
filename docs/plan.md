# Conversation: Creating a Sentry SDK for GameMaker (using Sentry Native)

---

**User:**  
I want to create a sentry SDK for gamemaker. I want to use sentry native under the hood. Come up with very high level overview of the steps needed to do.  
This should be a markdown file that's not more than 1 page and just outlining the steps taken so I can further analyze the effort needed.

**Assistant:**  
Here’s a compact high-level overview in markdown format:

```markdown
# High-Level Overview: Sentry SDK for GameMaker (using Sentry Native)

## 1. Understand Requirements

X Define scope: crash reporting, breadcrumbs, performance, etc.
X Confirm supported platforms (Windows, macOS, Linux, consoles?).
X Review GameMaker’s extension system (GML ↔ native bindings).

## 2. Set Up Sentry Native

X Fetch and build [sentry-native](https://github.com/getsentry/sentry-native) for target platforms.
X Verify integration works in a minimal C/C++ application.

## 3. Create GameMaker Extension Layer

- Use GameMaker’s **native extension interface** (DLL/.so/.dylib).
- Expose C functions that wrap key Sentry Native APIs:
  - Initialization (`sentry_init`)
  - Capture event/exception
  - Add breadcrumb
  - Set context/tags/user
  - Flush/close

## 4. Bridge to GML

- Create GML scripts that call the native extension functions.
- Map GameMaker datatypes → C datatypes (strings, numbers, JSON).
- Provide a clean GML API surface (e.g. `sentry_init(config)`, `sentry_capture(msg)`).

## 5. Packaging & Distribution

- Package extension files (compiled binaries + GML scripts) as a GameMaker extension.
- Provide platform-specific binaries inside the package.

## 6. Testing & Validation

- Test crash handling (null deref, divide by zero).
- Validate event delivery to Sentry dashboard.
- Test breadcrumbs, tags, and contexts from GML.
- Verify behavior on all supported platforms.

## 7. Documentation & Examples

- Document installation steps (add extension, initialize, capture).
- Provide minimal GML example project with integration.

---

This outlines the major tasks for wrapping **Sentry Native** into a usable GameMaker SDK.
```

---

**User:**  
what language are game maker scripts in?

**Assistant:**  
GameMaker scripts are written in **GML (GameMaker Language)**.

- It’s a **proprietary, C-like scripting language** made for GameMaker.
- Syntax feels similar to JavaScript or C (with functions, arrays, structs, objects).
- It’s used for **game logic, scripts, events, and objects** inside GameMaker.
- You can also call into **native extensions** (C/C++ libraries, packaged as DLL/so/dylib) via the extension system.

---

**User:**  
does GML throw exceptions? can I capture those to sentry?

**Assistant:**  
Good question — this is where GML differs from languages like C++ or C#.

👉 **GML does not have traditional exceptions** (no `try/catch` or stack unwinding).

Instead, it has:

- **Error handling at runtime**: If something goes wrong (e.g. accessing an undefined variable, calling a function with the wrong arguments), GameMaker will **throw a fatal error** that usually halts the game unless handled.
- **`show_error()`**: You can trigger a controlled error popup or abort from code.
- **Error events**: Some runtime errors (e.g. networking, JSON parsing) raise **asynchronous events** with error codes instead of exceptions.

### Capturing Errors in Sentry

Since GML itself doesn’t throw catchable exceptions:

1. **Custom Error Logging**

   - Wrap critical logic in checks.
   - On failure, manually call a `sentry_capture_message("Something went wrong")` or `sentry_capture_event({...})`.

2. **Fatal Error Capture (Native Side)**

   - If the game crashes (null deref, invalid memory access, etc.), **Sentry Native** can capture that crash at the C level.
   - That means you _can_ get crash reports even if GML doesn’t expose the error directly.

3. **Bridging Errors From GML → Native**
   - When you detect a logic failure in GML, you can send it as a **breadcrumb** or **event** to Sentry through your extension.
   - This acts like a lightweight replacement for exceptions.

⚠️ In short:

- **Crashes & low-level errors → Sentry Native will capture automatically.**
- **GML runtime/logic errors → You must explicitly report them via your extension.**

---

**User:**  
there's exception_unhandled_handler, can't I just pass sentry's sentry_capture_event for that

**Assistant:**  
Ah, yes — you’re right 👍.

GameMaker **does** have the function [`exception_unhandled_handler`](https://manual.gamemaker.io/monthly/en/GameMaker_Language/GML_Reference/Debugging/exception_unhandled_handler.htm).

Here’s how it works:

- You can register a GML function to be called whenever an **unhandled exception** occurs.
- That function receives a struct with details like:
  - `exception_type`
  - `exception_message`
  - `exception_longMessage`
  - `script_name`
  - `line`
  - `stacktrace`

That means you **can capture these exceptions** and forward them to Sentry.

### How you’d wire it up

```gml
function my_sentry_handler(exc) {
    var payload = {
        "type": exc.exception_type,
        "message": exc.exception_message,
        "long": exc.exception_longMessage,
        "script": exc.script_name,
        "line": exc.line,
        "stack": exc.stacktrace
    };

    // Call into your native extension wrapper
    sentry_capture_event(payload);
}

// Register at game start
exception_unhandled_handler(my_sentry_handler);
```

On the **native side**, your extension would serialize that struct (probably as JSON) and pass it into `sentry_capture_event` from **Sentry Native**.

✅ So yes — by hooking into `exception_unhandled_handler`, you _can_ capture GML exceptions and automatically send them to Sentry.

---
