# Custom Unix Shell

A fully functional Unix shell implemented in C, supporting **process management, job control, signal handling, redirection, piping, and command history**. 

## Features 🚀
- **Process Creation & Execution** (`fork()`, `exec()`, `wait()`)
- **Foreground & Background Process Management** (`&` support)
- **Signal Handling** (`SIGCHLD`, `SIGINT`, `SIGTSTP`)
- **Built-in Commands** (`cd`, `exit`)
- **Input & Output Redirection** (`<`, `>`)
- **Piping** (`|`)
- **Command History** (using `readline` library)
- **Tab Auto-Completion** (via `readline`)

## Installation & Usage 🔧
### **Clone the Repository**
```sh
git clone https://github.com/your-username/custom-shell.git
cd custom-shell
```

### Compile the Shell
``` bash
gcc -o custom_shell custom_shell.c -lreadline
```

### Run the Shell
``` bash
./custom_shell
```

### Usage Examples

✅ Running Basic Commands
``` bash
custom-shell$ ls -l
custom-shell$ echo "Hello, Unix Shell!"
```
✅ Background Execution
``` bash
custom-shell$ sleep 10 &
[Background Job] PID: 12345
```
✅ Job Control (Foreground & Background)
``` bash
custom-shell$ fg 12345   # Brings process 12345 to foreground
custom-shell$ bg 12345   # Resumes process 12345 in background
```
✅ Input & Output Redirection
``` bash
custom-shell$ cat < input.txt
custom-shell$ ls > output.txt
```
✅ Piping
``` bash
custom-shell$ ls | grep ".c" | wc -l
```
✅ Built-in Commands
``` bash
custom-shell$ cd /home/user
custom-shell$ exit
```
✅ Command History & Auto-Completion
``` bash
- Use Arrow keys (Up/Down) to browse history.
- Type part of a command and press Tab for auto-completion.
```

### Implementation Details
1️⃣ Process Creation & Execution
- Uses fork() to create child processes.
- Executes commands using execvp().
- Waits for foreground processes using waitpid().
  
2️⃣ Signal Handling
- SIGINT (Ctrl+C): Terminates foreground processes.
- SIGTSTP (Ctrl+Z): Sends process to background.
- SIGCHLD: Handles background process termination.
  
3️⃣ Redirection (<, >)
- Redirects standard input/output using dup2() and open().
  
4️⃣ Piping (|)
- Implements inter-process communication via pipe().
- Spawns multiple child processes and connects input/output streams.
  
5️⃣ Built-in Commands
- cd (change directory)
- exit (terminate shell)

### License 📜
This project is licensed under the MIT License.

### 👨‍💻 Author
Srinath Duvvuri
📧 duvvurisrinath@gmail.com
