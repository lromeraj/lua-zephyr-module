# Lua Zephyr Module (WIP)

> **NOTE**: We are currently updating this repository for compatibility with the latest versions of Zephyr. New features will be added in the following days.

---

This repository provides an example of how the [Lua language](https://www.lua.org/about.html) can be integrated in a project using the [Zephyr embedded operating system](https://www.zephyrproject.org/). The repository is structured as a Zephyr module to simplify integration.


# Features (WIP)

- Fully integrated within the Zephyr's build system.
- A Lua compiler is built in a per board basis so no runtime problems can arise due to configuration mismatch.
- Support Lua byte code embedded directly into the application firmware.
- Support for the [luasocket](https://github.com/lunarmodules/luasocket) library.

# Application examples

Under the folder `samples/`, you can find examples of how to use Lua in a Zephyr project. Lua scripts are compiled using `luac` and embedded directly into the firmware. This approach avoids the need for a filesystem, making it lightweight and suitable for embedded environments.

- `samples/app`: Basic Lua application with a countdown loop.
- `samples/socket`: Test luasocket library running a simple TCP server. 

# Building

The Lua files are compiled and embedded directly into the firmware. The build system is integrated with Zephyr’s `CMake` process. A helper function `lua_generate_inc_file_for_target` is used to simplify building:

```cmake
generate_lua_inc_file_for_target(
  app
  ${CMAKE_CURRENT_SOURCE_DIR}/lua/main.lua
)
```

Lua source files are located under `lua/` in the corresponding application folder. Any modification to these files triggers a rebuild of the final binary firmware just like changes in `.c` or `.h` files.

Build the firmware using the `west` tool from the `app/` directory:

```bash
west build -b <BOARD>
```

If you do not have a physical board, you can use Qemu. The recommended board is `qemu_x86`:

```bash
west build -b qemu_x86
```

We also provide a debug configuration file which can be used at build time:

```bash
west build -b qemu_x86 -- -DOVERLAY_CONFIG=dbg.conf
```

# Running the app

To run the application in Qemu:
```bash
west build -t run
```

> **NOTE**: Use `CTRL+A` then `C` to interact with the Qemu virtual machine.