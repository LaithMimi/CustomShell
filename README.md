# CustomShell

## Overview

This project consists of two main C programs (`ex1.c` and `ex2.c`) that implement custom shell functionalities, including command execution, alias management, and job control. The project is configured using CMake for easy building and compilation.

## Files

- **ex1.c**: Implements alias management, command execution, and script file handling.
- **ex2.c**: Focuses on job control and command execution, managing background jobs and handling command operators (`&&`, `||`).
- **CMakeLists.txt**: CMake configuration file for building the project.

## Installation

To build this project, you need to have CMake installed. Follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/LaithMimi/OP_Ex1.git
   cd OP_Ex1
   ```

2. Create a build directory and navigate to it:
   ```sh
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:
   ```sh
   cmake ..
   ```

4. Build the project:
   ```sh
   make
   ```

## Usage

After building the project, you will have an executable named `Ex1`. You can run this executable to start the custom shell:

```sh
./Ex1
```

### ex1.c Features

- **Alias Management**: Define (`alias`) and remove (`unalias`) command aliases.
- **Command Execution**: Execute system commands, including handling quotes and special characters.
- **Script File Execution**: Execute commands from script files with `.sh` extension.

### ex2.c Features

- **Job Control**: Manage background jobs, display running jobs, and handle job completion.
- **Command Operators**: Support for `&&` and `||` operators for conditional command execution.
- **Error Redirection**: Redirect error output to specified files using `2>` syntax.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

