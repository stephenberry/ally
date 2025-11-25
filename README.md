# Ally Plugin System

A lightweight plugin system for C and C++ with a pure C interface. Load shared libraries and expose structured interfaces with minimal boilerplate.

## Features

- **Pure C Interface** - Core API is pure C for maximum compatibility
- **C++ Wrapper** - Optional RAII wrapper for C++ convenience
- **Cross-Platform** - Works on Windows, Linux, and macOS
- **Simple API** - Load plugins in 3 lines of code

## Architecture

1. **`ally.h`** - Core library header (pure C) providing generic plugin loading functions
2. **`ally_interface.h` or `ally_interface.hpp`** - User-provided headers that define namespaced types for each plugin domain
3. **Plugin implementation** - Exports a factory function `ally_get_interface()` returning the plugin interface

Each plugin type uses unique names to avoid collisions, allowing multiple different plugin types in one application:

- **calculator** plugin: `ally_calculator_type`, `ally_calculator_version`
- **renderer** plugin: `ally_renderer_type`, `ally_renderer_version`

Plugin names must be derived from the shared library filename. Version information is provided via the namespaced constant.

## Quick Start

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Optional CMake flags:
- `-DBUILD_EXAMPLES=OFF` - Skip example plugins and hosts
- `-DBUILD_TESTS=OFF` - Skip unit tests
- `-DBUILD_SHARED_LIBS=OFF` - Build as static library

This builds:
- `libally` - Core plugin loading library
- Example plugins (calculator, string processor)
- Example hosts (C and C++)

### Running Examples

```bash
# Run C host with calculator plugin
./c_host ./calculator_plugin

# Run C++ host with string processor plugin
./cpp_host ./string_processor_plugin
```

## Creating a Plugin

### Step 1: Define Your Interface

Create `ally_interface.h` (for C) or `ally_interface.hpp` (for C++):

**C Version (`ally_interface.h`):**

```c
#ifndef ALLY_INTERFACE_H
#define ALLY_INTERFACE_H

#include "ally/ally.h"

/* Plugin version - namespaced and required */
static const char* const ally_my_plugin_version = "1.0.0";

typedef struct my_plugin {
    /* Your plugin API */
    int (*add)(int a, int b);
    void (*hello)(void);
} my_plugin;

/* Namespaced type allows multiple plugin types in one app */
typedef my_plugin ally_my_plugin_type;

static inline ally_my_plugin_type* ally_get_my_plugin(ally_handle_t handle) {
    return (ally_my_plugin_type*)ally_get(handle);
}

#endif
```

**C++ Version (`ally_interface.hpp`):**

```cpp
#pragma once

#include "ally/ally.h"
#include <string>

/* Plugin version - namespaced and required */
static const char* const ally_my_plugin_version = "1.0.0";

struct my_plugin {
    virtual ~my_plugin() = default;

    virtual std::string process(const std::string& text) = 0;
    virtual int calculate(int lhs, int rhs) = 0;
    virtual void log(const std::string& msg) = 0;
};

using ally_my_plugin_type = my_plugin;

inline ally_my_plugin_type* ally_get_my_plugin(ally_handle_t handle) {
    return static_cast<ally_my_plugin_type*>(ally_get(handle));
}
```

### Version String Format

Every plugin must define a namespaced `ally_<plugin>_version` constant, and the string must be in `<major>.<minor>.<patch>` form (numeric components separated by dots). Keep it ASCII, omit prefixes like `v1.2.3`, and avoid suffixes such as `-beta` so that hosts can compare versions with simple parsing logic. Example: `static const char* const ally_shader_version = "2.5.1";`.

### Step 2: Implement Your Plugin

**C Implementation:**

```c
#include "ally/ally.h"

static int add_impl(int a, int b) {
    return a + b;
}

static void hello_impl(void) {
    printf("Hello from plugin!\n");
}

static ally_my_plugin_type plugin_instance = {
    .add = add_impl,
    .hello = hello_impl
};

/* Export using standardized symbol name */
ALLY_EXPORT ally_my_plugin_type* ALLY_CALL ally_get_interface(void) {
    return &plugin_instance;
}
```

**C++ Implementation:**

```cpp
#include "ally/ally.h"
#include "ally_interface.hpp"
#include <iostream>

/* Concrete implementation of the plugin interface */
class my_plugin_impl : public my_plugin {
public:
    std::string process(const std::string& text) override {
        return "[plugin] " + text;
    }

    int calculate(int lhs, int rhs) override {
        return lhs * rhs;
    }

    void log(const std::string& msg) override {
        std::cout << "[plugin log] " << msg << "\n";
    }
};

static my_plugin_impl plugin_instance;

/* Export using standardized symbol name */
extern "C" ALLY_EXPORT ally_my_plugin_type* ALLY_CALL ally_get_interface(void) {
    return &plugin_instance;
}
```

### Step 3: Build Your Plugin

**CMake:**

```cmake
add_library(my_plugin SHARED my_plugin.cpp)
target_include_directories(my_plugin PRIVATE
    /path/to/ally/include
    ${CMAKE_CURRENT_SOURCE_DIR}  # For ally_interface.hpp
)
```

**Calculator C++ Plugin Example:**

```cmake
# calculator/CMakeLists.txt
add_library(calculator_plugin SHARED
    src/calculator_plugin.cpp        # Implements ally_get_interface
    include/ally_interface.hpp       # Defines ally_calculator_type/version
)

target_include_directories(calculator_plugin PRIVATE
    ${PROJECT_SOURCE_DIR}/include    # Ally headers (ally/ally.h, ally/ally.hpp)
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(calculator_plugin PROPERTIES
    OUTPUT_NAME "calculator_plugin"  # ally_load("./calculator_plugin") works cross-platform
)
```

A host project can then `ally_load("./calculator_plugin")` (or use `ally::plugin_loader<ally_calculator_type>`) and receive the calculator interface defined in `ally_interface.hpp`.

**Manual Compilation:**

```bash
# C plugin
gcc -shared -fPIC -o libmy_plugin.so my_plugin.c -I/path/to/ally/include

# C++ plugin
g++ -shared -fPIC -o libmy_plugin.so my_plugin.cpp -I/path/to/ally/include
```

## Using Plugins

### C Host Application

```c
#include "ally_interface.h"  /* Provides ally_get_my_plugin() helper */
#include "ally/ally.h"

int main(void) {
    /* Load the plugin (uses standardized ally_get_interface symbol) */
    ally_handle_t handle = ally_load("./my_plugin");

    if (!handle) {
        fprintf(stderr, "Error: %s\n", ally_last_error());
        return 1;
    }

    /* Get the interface via helper to avoid manual casts */
    ally_my_plugin_type* plugin = ally_get_my_plugin(handle);

    /* Use the plugin */
    printf("Plugin version: %s\n", ally_my_plugin_version);
    int result = plugin->add(5, 3);
    plugin->hello();

    /* Unload when done */
    ally_unload(handle);

    return 0;
}
```

**Compile:**

```bash
gcc -o my_host my_host.c -I/path/to/ally/include -L/path/to/build -lally
```

**CMake (recommended for larger hosts):**

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_host C)

find_package(ally CONFIG REQUIRED)  # Provides ally::ally

add_executable(my_host src/my_host.c)
target_link_libraries(my_host PRIVATE ally::ally)
target_include_directories(my_host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

After installing Ally (`cmake --install build --prefix /opt/ally`), point CMake at it with
`cmake -Dally_DIR=/opt/ally/lib/cmake/ally ..` and the imported `ally::ally` target will
provide the correct include path and linker flags automatically.

### C++ Host Application

```cpp
#include "ally_interface.hpp"  /* Your plugin interface */
#include "ally/ally.hpp"

int main() {
    /* Load the plugin using C++ wrapper with template parameter */
    ally::plugin_loader<ally_my_plugin_type> loader;

    if (!loader.load("./my_plugin")) {
        std::cerr << "Error: " << loader.last_error() << "\n";
        return 1;
    }

    /* Display plugin info */
    std::cout << "Plugin version: " << ally_my_plugin_version << "\n";

    /* Use arrow operator for convenient access */
    std::string result = loader->process("test");
    int value = loader->calculate(6, 7);

    /* Plugin automatically unloaded when loader goes out of scope */
    return 0;
}
```

**Compile:**

```bash
g++ -o my_host my_host.cpp -I/path/to/ally/include -L/path/to/build -lally
```

**CMake example:**

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_cpp_host CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ally CONFIG REQUIRED)

add_executable(my_cpp_host src/main.cpp)
target_link_libraries(my_cpp_host PRIVATE ally::ally)
target_include_directories(my_cpp_host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces  # Folder containing ally_interface.hpp
)
```

This setup works both when Ally is installed system-wide and when you vendor the
project via `add_subdirectory()`. In the latter case, replace `find_package` with:

```cmake
add_subdirectory(external/ally)
target_link_libraries(my_cpp_host PRIVATE ally)
```

### Loading Multiple Plugin Types

The namespace-based design allows loading different plugin types simultaneously:

```cpp
#include "calculator/ally_interface.h"
#include "renderer/ally_interface.hpp"
#include "ally/ally.h"
#include "ally/ally.hpp"

int main() {
    /* Load calculator plugin (C interface) */
    ally_handle_t calc_h = ally_load("./calculator");
    ally_calculator_type* calc = ally_get_calculator(calc_h);

    /* Load renderer plugin (C++ interface) */
    ally::plugin_loader<ally_renderer_type> renderer;
    renderer.load("./renderer");

    /* Use both plugins - no namespace collisions! */
    printf("Calculator version: %s\n", ally_calculator_version);
    std::cout << "Renderer version: " << ally_renderer_version << "\n";

    double result = calc->add(2, 3);
    renderer->draw_frame();

    ally_unload(calc_h);
    return 0;
}
```

## API Reference

### C API (`ally.h`)

#### `ally_handle_t ally_load(const char* plugin_path)`

Load a plugin from a shared library using the standardized symbol `ally_get_interface`.

- **plugin_path**: Path to plugin (can omit extension, e.g., `"./my_plugin"`)
- **Returns**: Plugin handle, or `NULL` on failure

The loader automatically tries platform-specific variants:
- Exact path
- Path + extension (`.so`, `.dylib`, `.dll`)
- Path with `lib` prefix + extension (Unix only)

All Ally plugins must export the standardized factory function name: `ally_get_interface`

#### `void* ally_get(ally_handle_t handle)`

Get the plugin interface as a void pointer.

- **Returns**: Pointer to plugin interface (void*), or `NULL` if invalid
- **Note**: Cast to your plugin type or use type-safe helpers like `ally_get_my_plugin()`

#### `void ally_unload(ally_handle_t handle)`

Unload a plugin and free resources.

#### `bool ally_is_loaded(ally_handle_t handle)`

Check if plugin is currently loaded.

#### `const char* ally_last_error(void)`

Get error message from last failed operation.

#### `const char* ally_plugin_path(ally_handle_t handle)`

Get full path to loaded plugin library.

### C++ API (`ally.hpp`)

#### `template<class PluginType> struct ally::plugin_loader`

RAII wrapper for plugin management. Template parameter specifies the plugin interface type.

**Methods:**
- `bool load(path)` - Load a plugin (uses standardized `ally_get_interface` symbol)
- `void unload()` - Manually unload (called automatically)
- `PluginType* get() const` - Get plugin interface (type-safe)
- `bool is_loaded() const` - Check if loaded
- `std::string last_error() const` - Get last error
- `std::string plugin_path() const` - Get plugin path

**Operators:**
- `operator->()` - Access plugin members (type-safe)
- `operator*()` - Dereference (type-safe)
- `operator bool()` - Check if loaded

**Example:**
```cpp
ally::plugin_loader<ally_calculator_type> loader;
loader.load("./calculator");
double result = loader->add(2, 3);  // Type-safe!
```

#### `template<class PluginType> ally::plugin_loader<PluginType> ally::load_plugin(path)`

Helper function that throws `std::runtime_error` on failure.

Loads a plugin using the standardized `ally_get_interface` symbol.

## Platform-Specific Notes

### Linux

Plugins are named `libname.so`:

```bash
gcc -shared -fPIC -o libmy_plugin.so my_plugin.c
```

Load with: `ally_load("./my_plugin")`

### macOS

Plugins are named `libname.dylib`:

```bash
gcc -shared -fPIC -o libmy_plugin.dylib my_plugin.c
```

Load with: `ally_load("./my_plugin")`

### Windows

Plugins are named `name.dll`:

```bash
cl /LD my_plugin.c /Fe:my_plugin.dll
```

Load with: `ally_load("./my_plugin")`

## Examples

The repository includes complete examples:

### C Calculator Plugin

- **Interface**: `examples/c_plugin/ally_interface.h`
- **Implementation**: `examples/c_plugin/calculator_plugin.c`
- **Features**: Basic arithmetic, memory storage

### C++ String Processor Plugin

- **Interface**: `examples/cpp_plugin/ally_interface.hpp`
- **Implementation**: `examples/cpp_plugin/string_processor_plugin.cpp`
- **Features**: String transformations, analysis, history tracking

### Host Applications

- **C Host**: `examples/hosts/c_host.c` - Loads calculator plugin
- **C++ Host**: `examples/hosts/cpp_host.cpp` - Loads string processor plugin

## Design Philosophy

### Why User-Defined `ally_type`?

- **Type Safety**: Each plugin declares its own interface type
- **Flexibility**: Interface can be any struct type (C or C++)
- **Compile-Time Checks**: Type mismatches caught at compile time
- **Zero Overhead**: No runtime type information needed

### Why Static Instances?

Plugins return pointers to static instances because:
- Simple and efficient
- No memory management required
- Plugins typically have singleton behavior
- If you need multiple instances, add a `create()` method to your interface

## Advanced Usage

### Multiple Plugin Instances

If you need multiple instances, add factory methods to your interface:

```cpp
struct my_plugin {
    /* Factory method */
    void* (*create_instance)(void);
    void (*destroy_instance)(void*);

    /* Instance methods take void* as first argument */
    void (*process)(void* instance, const char* data);
};
```

### Plugin Dependencies

Plugins can expose functions to inject host capabilities:

```c
struct my_plugin {
    /* Host sets these before calling init */
    void (*host_log)(const char* message);
    uint64_t (*host_get_time)(void);

    /* Plugin calls this after host sets dependencies */
    void (*init)(void);
};
```

### Version Checking

Use the namespaced static constants for version info:

```c
/* In ally_interface.h */
static const char* const ally_my_plugin_version = "1.0.0";

struct my_plugin {
    /* ... interface functions ... */
};

typedef my_plugin ally_my_plugin_type;

/* In host code */
#include <string.h>
#include "ally_interface.h"
static const char* const EXPECTED_VERSION = "1.0.0";

if (strcmp(ally_my_plugin_version, EXPECTED_VERSION) != 0) {
    fprintf(stderr, "Version mismatch (host %s, plugin %s)\n",
            EXPECTED_VERSION, ally_my_plugin_version);
}
printf("Plugin version: %s\n", ally_my_plugin_version);
```

The plugin name can be derived from the shared library filename or added as a field in your struct if needed.

## Troubleshooting

### Plugin fails to load

**Error:** "Failed to load plugin"

- Check that the file exists and path is correct
- Verify library naming (lib prefix, extension)
- Check library search path (`LD_LIBRARY_PATH`, `DYLD_LIBRARY_PATH`, `PATH`)
- Use `ally_plugin_path()` to see what path was tried

### Symbol not found

**Error:** "Failed to find symbol 'ally_get_interface'"

- Ensure you used `extern "C" ALLY_EXPORT ally_type* ALLY_CALL ally_get_interface(void)`
- Verify your plugin exports the standardized symbol name: `ally_get_interface`
- Check exported symbols:
  - Linux/macOS: `nm -D libmy_plugin.so | grep ally_get_interface`
  - Windows: `dumpbin /EXPORTS my_plugin.dll`

### Type mismatch / Segfault

**Cause:** Host and plugin have different definitions of `ally_type`

- Ensure host includes the **exact same** `ally_interface.h/hpp` as plugin
- Verify C/C++ standard matches (e.g., both use C++17)
- Check compiler ABI compatibility

## License

MIT License - see LICENSE file for details.
