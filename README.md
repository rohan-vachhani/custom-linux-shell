# Custom Linux Shell  

## Overview  
This project is a **custom command-line shell** built using C and Linux system calls. It provides an interactive interface for executing built-in Linux commands and supports advanced shell functionalities such as command chaining, output redirection, and piping.  

## Features  
- **Command Execution:** Runs built-in Linux commands using `fork()`, `exec()`, and `wait()`.  
- **Directory Navigation:** Implements the `cd` command to change the working directory.  
- **Error Handling:** Displays error messages for incorrect commands while maintaining shell stability.  
- **Signal Handling:** Gracefully handles `Ctrl+C` and `Ctrl+Z` without terminating the shell.  
- **Command Chaining:**  
  - **Sequential execution (`##`)**: Commands execute one after another.  
  - **Parallel execution (`&&`)**: Commands execute simultaneously.  
- **Output Redirection:** Redirects command output to a file using `>`.  
- **Piping (`|`):** Enables output of one command to be used as input for another (e.g., `ls | grep .c | wc -l`).  

## Installation  
To set up and run the shell on your system:  

1. **Clone the repository:**  
   ```bash
   git clone https://github.com/rohan-vachhani/custom-linux-shell.git  
   cd custom-linux-shell  
2. **Compile the shell:**
    ```bash
    gcc myshell.c -o myshell  
3. **Run the shell:**
    ```bash
    ./myshell  

## Usage
- **Run commands:** Type any valid Linux command and press Enter.
- **Change directory:** Use `cd <directory>` or `cd ..`.
- **Exit shell:** Type `exit`.
- **Execute multiple commands:**
  - Sequential:
    ```bash
    ls ## pwd ## whoami
  - Parallel:
    ```bash
    ls && pwd && whoami
- **Redirect output:**
  ```bash
  ls > output.txt
- **Use pipes:**
  ```bash
  cat file.txt | grep error | wc -l
  
## Future Improvements
- Support background process execution (&).
- Add environment variable support (export, $VAR).
