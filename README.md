# SakiGit

An AI-powered Git GUI client that provides a visual interface for daily Git operations, integrates large language models, and supports intelligent features such as auto-generating commit messages and smart code review, thereby lowering the barrier to version control and improving development efficiency.

## Requirements

Before you begin, ensure your development environment meets the following requirements:

| Component | Version / Specification |
| :--- | :--- |
| **Operating System** | Windows 10 / 11 |
| **Qt** | 6.10.0 or later |
| **Compiler** | MinGW 13.1.0 (x86_64) |
| **Build System** | CMake 3.16 or later |
| **Build Generator** | Ninja (recommended) or Make |
| **Git** | (Optional, for version control) |

> 📌 **Notes**:
> - This project uses **CMake** as its build system. Ensure CMake is installed and added to your system `PATH`.
> - It is recommended to install Qt 6.10.0 and the corresponding MinGW toolchain via the **Qt Maintenance Tool**.
> - If you use a different Qt version or compiler, you may need to adjust the relevant settings in `CMakeLists.txt`.

---

## Installation & Build Steps

Follow these steps to build and run the project from source.

### 1. Clone the repository

```bash
git clone https://github.com/yourname/SakiGit.git
cd SakiGit
```

### 2. Configure and build (using CMake + Ninja)

We recommend creating a separate `build` directory outside the source root to keep things clean:

```bash
# Create build directory
mkdir build
cd build

# Run CMake configuration
cmake -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64" ..

# Start building (Release configuration)
cmake --build . --config Release
```

> 💡 **Explanation**:
> - `-G "Ninja"` tells CMake to use Ninja as the build generator, which speeds up compilation.
> - `-DCMAKE_PREFIX_PATH` points to your Qt installation directory so CMake can find the Qt libraries. Adjust this path to match your actual Qt installation location.

### 3. Run the application

After a successful build, the executable will be placed inside the build directory:

```bash
# Navigate to the output folder (if needed)
cd bin  # or directly run the executable
./SakiGit.exe
```

### 4. (Optional) Open with Qt Creator

If you prefer a graphical interface:

1. Open **Qt Creator**.
2. Click **File -> Open File or Project** and select the `CMakeLists.txt` file in the project root.
3. When choosing a kit, make sure to select **Desktop Qt 6.10.0 MinGW 64-bit**.
4. Click **Configure Project**, then click the **green triangle** (Run) in the lower left corner to start the application.

---

### 📌 Common Issues

- **Qt libraries not found**: Verify that `CMAKE_PREFIX_PATH` points to the correct Qt installation folder.
- **Compiler mismatch**: Ensure the MinGW version you use matches the one your Qt libraries were built with (both should be 13.1.0).
- **Ninja not found**: You can use the Ninja that comes with Qt Creator (usually located in `C:/Qt/Tools/Ninja`) or download it yourself and add it to your `PATH`.
