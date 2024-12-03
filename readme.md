# Project Build and Usage Guide

## Install project

To install the project, clone the repository and its submodules:

1. Clone the repository:
    ```bash
    git clone git@github.com:RRevkoUA/cpp-archiver.git
    ```

2. Navigate to the project directory:
    ```bash
    cd cpp-archiver
    ```

3. Initialize and update submodules:
    ```bash
    git submodule update --init --recursive
    ```

## Building the Project

To build the project, follow these steps:

1. Create a build directory if it doesn't exist:
    ```bash
    mkdir -p build
    ```

2. Navigate to the build directory:
    ```bash
    cd build
    ```

3. Configure the project using CMake:
    ```bash
    cmake ..
    ```

4. Build the project:
    ```bash
    make -j4
    ```

## Using the Project

The project provides functionality to compress and extract files. Below are the commands to use these features:

### Compress a File or Directory

To compress a file or directory, use the `--compress` or `-c` option followed by the source file/directory and optionally the archive name:

```bash
./archiver --compress <source> [archive]
```

For example, to compress the `test.txt` file into an archive named `test.txt.tar.zip`, you would use the following command:

```bash
./archiver --compress <path to test.txt> <path to test.txt.tar.zip> 
```
or 

```bash
./archiver --compress <path to test.txt> -t zip
```

### Extract an Archive

To extract an archive, use the `--extract` or `-e` option followed by the archive file and optionally the destination directory:

```bash
./archiver --extract <archive> [destination]
```

For example, to extract the `test.txt.tar.zip` archive into the `extracted` directory, you would use the following command:

```bash
./archiver --extract <path to test.txt.tar.zip> <path to extracted dir>
```

### Specify Compression Type

You can specify the compression type using the `--type` option. 

```bash
./archiver (-c/-e ...)  --type <type>
```

## Running Tests

### Automatic Testing

To run tests automatically, you can use the `test.sh` script. This script will prepare the test files, build the project, and run the tests.

```bash
./test.sh
```

### Manual Testing

If you prefer to run tests manually, follow these steps:

1. Prepare the test files:
    ```bash
    cmake --build . --target PrepareTestFiles
    ```

2. Run the tests using CTest:
    ```bash
    ctest -R "^s_test_|^f_test_"
    ```

## Pros and Cons

### Using the Script

**Pros:**
- Automates the entire process, reducing the chance of human error.
- Ensures that all required steps are performed in the correct order.
- Saves time by combining multiple commands into a single script.
- Eliminates files created during testing

**Cons:**
- Less control over individual steps.
- More difficult to debug if something goes wrong in the script.

### Manual process

**Pros:**
- Full control over each step of the process.
- Easier to debug specific issues as they arise.

**Cons:**
- More time-consuming and prone to human error.
- Requires memorizing and executing multiple commands in the correct order.