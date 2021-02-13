# Building with Powershell

This is my preferred approach, using no IDE.

## 1. Install Chocolatey

## 2. Install Build Tools

Install build tools using chocolatey (if not already installed).

```powershell
choco install git cmake mingw
```

## 3. Choose install location

Change to the directory that you would like to contain the source code.

For the rest of this guide, we will assume you are using your "Documents"
directory.

## 4. Clone and Build PDCurses

```powershell
git.exe clone https://github.com/wmcbrine/PDCurses
```

## 5. Clone and Build Tetris

Clone https://github.com/wmcbrine/PDCurses

```powershell
git.exe clone https://github.com/nitepone/terminally-tetris
```

Clone submodules

```powershell
git.exe submodule init
git.exe submodule update
```

Set the path to the PDCurses repo using an environmental variable.

Generate our Makefile using CMake. In the command below, replace
`C:/Users/elliot/Documents/PDCurses` with the location that you cloned
PDCurses. Note that *forward slashes* must be used here to comply with cmake.

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -G "MinGW Makefiles" -D "PDCURSES_REPO_DIR=C:/Users/elliot/Documents/PDCurses" .
```

*Note: Make sure you type the above command exactly as written. The amperstand
tells powershell to treat the quoted path as a program and execute it.*

*Note: We have to explicitly name the generator we want, in this case
"MinGW Makefiles"*

Run make

```powershell
make
```

## 6. [Optional] Run Tests

```powershell
.\bin\unit_tests.exe
```

*Note: Some tests (that rely on a specific Linux `rand_r` seeding) will fail on
Windows. Hopefully, we will fix this in the future.*

### 7. [Optional] Run the server

If you want to play online, start a server.

```powershell
.\bin\server.exe
```

### 8. Run the Tetris Client Application

```powershell
.\bin\client.exe
```

# Formatting Code with Clang Format

First, make sure you have chocolatey installed as described under "Building
with Powershell".

Install LLVM Tools

```powershell
choco install LLVM
```

Change to the root directory of this repo, wherever you cloned it.

Run clang-format.

```powershell
& "C:\Program Files\LLVM\bin\clang-format.exe" -style=file -i src/*.c src/*.h
```

*Note: Make sure you type the above command exactly as written. The amperstand
tells powershell to treat the quoted path as a program and execute it.*

# Appendix

## PDCurses Shortfalls

### Missing Menu and Forms

PDCurses does not come with menu and forms like curses does. We may need to
rewrite some code to get around this.

## Considered Alternatives

### Cygwin

The main advantage of Cygwin is that it offers a greater degree of
POSIX-compliance, which might allow us to use fewer pre-processing directives
to support Windows.

### LLVM

LLVM seems like it could be a good way to go, although *PDCurses does not come
with an out-of-the-box* Makefile for clang, and after overriding `CC` in the
`GCC` makefile, I found that a number of libaries were missing. We could look
into this more in the future, but it seemed like more work.

## C Package Managers

- Open Question: Is there a better way to link PDCurses, or are we stuck
  compiling it ourselves?
  - Could we use NuGet?
