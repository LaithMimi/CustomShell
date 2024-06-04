# Custom Shell with Aliases

## Overview

This project implements a custom shell that supports command execution,
directory navigation, and aliasing of commands. The shell allows users to define, list, and remove aliases,
execute commands from scripts, and navigate the file system using the `cd` command.
Additionally, the shell supports an `exit_shell` command to terminate the shell.

## Features

- **Command Execution**: Execute system commands directly from the shell.
- **Directory Navigation**: Change the current working directory using the `cd` command.
- **Aliases**: Define and manage command aliases for easier command execution.
  - **Define Alias**: Use `alias name='command'` to create an alias.
  - **List Aliases**: Use `alias` to list all defined aliases.
  - **Remove Alias**: Use `unalias name` to remove an alias.
- **Script Execution**: Execute commands from a script file using the `source filename` command.
- **Exit Command**: Use `exit_shell` to terminate the shell.

## Getting Started

### Prerequisites
- Unix based System
- A C compiler (e.g., GCC)
- Make sure you have the necessary permissions to run and execute scripts on your system.

### Installation

1. Clone the repository:
   git clone https://github.com/LaithMimi/Ex1

2. Navigate to the project directory:
   cd Ex1

3. Compile the source code:
   gcc -o ex1 ex1.c 


### Usage

1. Run the shell:
   ./ex1

2. Use the shell commands:
   - To execute a command:
     command [args...]
 
   - To change the directory:
     cd <directory>

   - To define an alias:
     alias name='command'

   - To list all aliases:
     alias

   - To remove an alias:
     unalias name

   - To execute a script file:
     source <script_file>

   - To exit the shell:
     exit_shell

### Examples

1. Define and use an alias:
   alias greet='echo Hello, World!'
   greet
   # Output: Hello, World!


2. Execute a script file:
   source script.sh

3. Change directory:
   cd /path/to/directory

## Code Structure

- **main.c**: Contains the main function and core logic for the custom shell.
- **Alias Management**: Functions for defining, listing, and removing aliases.
- **Command Execution**: Functions for executing commands and handling built-in commands like `cd` and `exit_shell`.
- **Script Execution**: Function for executing commands from a script file.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- This project was A collage Excercise inspired by the need for a simple custom shell.
