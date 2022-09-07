# Home-brew Network Stack

Welcome to "Computer Networking". In this lab, we will build a network stack from scratch, starting from Layer 2 to Layer 4. We will see and learn how the network protocol operates, in particular how the TCP protocol works. We expect this lab takes you between 30 to 50 hours to complete.

![image info](image/feynman42.jpeg)

> What I can not create, I do not understand — Richard Feynman.

## Prerequisite

### Collaboration Policy

**All the assignments must be your own work.** *You* must write each line of code, except for the code we provide as a part of the assignments. Do not copy any code from other students. To prevent plagiarism, we will use a code structure-similarity detection tool to check all submissions. An exception is that code snippets that do not relate to the network protocol (e.g., [unlikely macros](https://stackoverflow.com/questions/109710/how-do-the-likely-unlikely-macros-in-the-linux-kernel-work-and-what-is-their-ben) and [unique pointer](https://en.cppreference.com/w/cpp/memory/unique_ptr)) are allowed to reuse.

**Discussion with others.** You can discuss high-level implementation ideas with other students. But again, DO NOT copy or look at anyone else's code.

### RFC-compliance

We aim to make our implementation RFC-compliant (that is, following the formal requirements for the protocols in the TCP/IP stack as specified in a number of RFCs). This lab handout document is intended to be a friendly guidebook rather than a formal specification. Therefore, we encourage you to read the following materials and strictly follow them to your best.

- Ethernet II frame format, [802.3-2018 - IEEE Standard](https://doi.org/10.1109%2FIEEESTD.2018.8457469). We recommend a quick overview from [Wikipedia](https://en.wikipedia.org/wiki/Ethernet_frame).
- Internet Protocol, [RFC791](https://www.rfc-editor.org/rfc/rfc791), in particular section 3.1.
- Transmission Control Protocol, [RFC793](https://www.rfc-editor.org/rfc/rfc793), in particular section 3.9.

### Useful Tools

- Git: distributed version control system. 
- CMake and Makefile: build automation, testing, packaging, and installation.
- [Wireshark](https://www.wireshark.org/): the world's foremost and widely-used network protocol analyzer.
- vnetUtils: a utility for creating and interacting with the virtual network on your machine. See `3rd/vnetUtils/README.md` for details.

## Quick Start

### Build All Targets

```
# Download from Github. You may create a fork and download it.
git clone https://github.com/SOAR-PKU/lab-netstack.git
cd lab-netstack

# Download submodules
git submodule init --recursive
git submodule update --recursive

# Build the targets. All built objects are in the `build` directory.
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Unit Test

Run all unit tests.

```
(cd build && ./runUnitTests)
```

Use a filter to run only the tests we specified.

```
(cd build && ./runUnitTests '--gtest_filter=SocketSessionTest.*')
```

### Optional: Code Formatting

This project uses clang-format-14 to format the source code. Follow https://apt.llvm.org/ to install. As an example for Ubuntu 20.04, use the following commands to install:

```
sudo add-apt-repository 'deb http://apt.llvm.org/focal/ llvm-toolchain-focal-14 main'
sudo apt update
sudo apt install -y clang-format-14
```

Use the `clang-format` CMake target to format the code in `/src` directory. Warning: this command permanently changes the code.

```
cmake --build build --target clang-format
```

### Optional: Stacktrace

This project uses Backward-cpp to print the stack trace. Install the following library to read the debug info:

```
sudo apt install -y binutils-dev
```

## Coding Guidance

Our code will be done in C++11. The provided code might be different from what you have written and read before. Here are some hints that we hope can help you understand our code as well as write better C++ style code.

### Asynchronous I/O

This protocol stack uses a single-thread non-blocking I/O framework. We use [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to enable non-blocking I/O. We strongly suggest you to read files in `src/base/` and make sure you understand them, especially `EpollAlarm` and `EpollServer`. `src/base/` also includes some utilities you will use in the following sections.

### Logging

We use [Google Logging framework](https://github.com/google/glog) to log and debug.

```cpp
// Log
LOG(INFO) << "Someting" << 42;
LOG(ERROR) << "Someting" << 42;

// If a line of code is executed many times, it may be useful to only log a
// message at certain intervals. This kind of logging is most useful for
// informational messages.
LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th packet";

// Log and kill the program
LOG(FATAL) << "Someting" << 42;

// Only output when compiled in debug mode (-DCMAKE_BUILD_TYPE=Debug)
DLOG(INFO) << "Someting" << 42;

// Sanity check in debug mode.
DCHECK(1 + 1 == 2);
```

### Testing

This protocol stack uses [Google Test](https://google.github.io/googletest/) for unit testing. See `utils/` for testing mocks and `ip_address_test.cpp` as an example. We encourage you to write your test suits but do not modify test suits that are parts of checkpoints.

### Memory Management

> C makes it easy to shoot yourself in the foot; C++ makes it harder, but when you do it blows your whole leg off. -- Bjarne Stroustrup

C leaves the memory allocation completely up to the developer; C++ develops constructors and destructors, [RAII](https://en.cppreference.com/w/cpp/language/raii), and smart pointers to help take care of the memory allocation. When talking about memory safety, people use the concept of "ownership" to denote who is responsible for freeing the memory that was allocated. Check [this article](https://belaycpp.com/2022/03/17/who-owns-the-memory/) to learn more about memory ownership. In this lab, we use `unique_ptr`s to mark the ownership and raw pointers to mark normal references.

## Assignment

In this lab, our goal is to complete the following tasks, each of which is marked as any one of PT (Programming Task), WT (Writing Task), or CP (Checkpoint).

- For the programming tasks, you should submit your implementation (additional documentation files are not necessary);
- For the writing tasks, you should submit your answer to each task;
- For each checkpoint, you should attach images, videos, or [typescript](https://man7.org/linux/man-pages/man1/script.1.html) files, along with a brief explanation.

### 1. Link Layer: Ethernet

#### WT1: Wireshark Warmup (1 pt)

Finish Exercise 1.2 of the guidebook. This gives you an intuitive overview of Ethernet II frames.

#### PT1: Link Layer Implementation (2 pts)

Insert your code to make the link layer work!

#### CP1: Link Layer IO (2 pts)

Show that your implementation can capture frames from a device and inject frames to a device using libpcap.

### 2. Network Layer: IP

#### WT2: Wireshark Warmup (1 pt)

Finish Exercise 2.1 in the guidebook. This gives you an intuitive overview of IP packets.

#### PT2: Network Layer Implementation (3 pts)

Insert your code to make the network layer work!a

#### CP2: Network Layer Test (2 pts)

Run unit tests and show that your implementation passes test suit `RoutingTableTest`.

#### CP3: Network Layer Functionality (4 pts)

Finish Exercises 2.2 and 2.3 in the guidebook. Run `router` in each node and show the results.

### 3. Transport Layer: TCP

#### WT3: Wireshark Warmup (1 pt)

Finish Exercise 3.1 of the guidebook. This gives you an intuitive overview of TCP segments.

#### PT3: Transport Layer Implementation (5 pts)

Insert your code to let the transport layer work!

#### CP4: Transport Layer Test (2 pts)

Run unit tests and show that your implementation passes test suit `SocketSessionTest`

#### CP5: Transport Layer Functionality (2 pts)

Create a virtual network with the following topology and run `echo_server` at ns4 and `echo_client` at ns1. Paste the output of them.
```
ns1 --- ns2 --- ns3 --- ns4
```

## Grading

These labs are worth 25 points (pts). Your gained points will be your final grade of lab 1, 2, 3 in this course. See points for each task above.

## Submission

You should submit your project in an archive named `lab[1|2|3]-[your name]-[your student ID].[tar|tar.gz|zip]`. In addition to the provided code, you should add:

- a file named `solution.pdf` containing your writing task solutions;
- a file named `not-implemented.pdf` listing tasks/features that are required but not implemented. You also need to explicitly give the reasons why you are not implementing them. Submit an empty file if you finished all the tasks. You may lose credits by adding items to this list, but being dishonest on this will make you lose more;
- a directory named `checkpoint/` containing your texts, images, videos, or typescript files for each checkpoint.

For each submission, please send an email with the title `lab[1|2|3] Name StudentID`
to zhouyuhan@pku.edu.cn. Missing the deadlines incurs a penalty.
