# ⚡ Parallel Data Pipeline on Linux

<div align="center">

```txt
██████╗  █████╗ ██████╗  █████╗ ██╗     ██╗     ███████╗██╗     
██╔══██╗██╔══██╗██╔══██╗██╔══██╗██║     ██║     ██╔════╝██║     
██████╔╝███████║██████╔╝███████║██║     ██║     █████╗  ██║     
██╔═══╝ ██╔══██║██╔══██╗██╔══██║██║     ██║     ██╔══╝  ██║     
██║     ██║  ██║██║  ██║██║  ██║███████╗███████╗███████╗███████╗
╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
```

### 🚀 High Performance Parallel Data Processing Pipeline

**Processes • Threads • Shared Memory • Semaphores • Named Pipes • Linux IPC**

</div>

---

# 🌌 Overview

Traditional sequential data processing systems waste CPU power by executing tasks one at a time. This project solves that problem using a **parallel Linux pipeline architecture** built with:

* ⚙️ Multi-processing
* 🧵 Multi-threading
* 🔄 IPC mechanisms
* 🔒 Synchronization primitives

The system processes large CSV datasets efficiently by splitting computation across independent pipeline stages.

---

# 🧠 System Architecture

```txt
+-----------+      +------------+      +-------------+      +-----------+
| Ingester  | ---> | Processor  | ---> | Shared Mem  | ---> | Reporter  |
+-----------+      +------------+      +-------------+      +-----------+
                         |
                 +----------------+
                 | Worker Threads |
                 +----------------+
```

---

# ⚡ Core Components

## 🖥️ Shell Controller

Responsible for:

* Runtime configuration
* Environment setup
* Validation of directories/files
* Cleanup using traps
* Execution monitoring

---

## 🎯 Dispatcher

Acts as the central coordinator.

### Responsibilities

* Creates child processes
* Initializes IPC resources
* Tracks runtime and exit status
* Coordinates synchronization between modules

---

## 📥 Ingester

Reads CSV data and streams chunks into the pipeline.

### Responsibilities

* Reads CSV files
* Splits data into chunks
* Streams data using named pipes
* Sends EOF signal after completion

---

## 🧵 Processor

Main computation engine powered by a worker thread pool.

### Features

* Parallel chunk processing
* Queue-based task distribution
* Aggregation table updates
* Thread-safe synchronization

### Synchronization Tools

* Mutexes
* Semaphores
* Poison Pill termination

---

## 📊 Reporter

Final stage of the pipeline.

### Responsibilities

* Reads aggregated data from shared memory
* Generates CSV reports
* Generates human-readable TXT reports
* Signals dispatcher after completion

---

# 🛠️ Tech Stack

| Technology    | Purpose                 |
| ------------- | ----------------------- |
| C++           | Core implementation     |
| Bash          | Runtime control         |
| POSIX Threads | Parallel processing     |
| Shared Memory | Fast IPC                |
| Named Pipes   | Streaming communication |
| Semaphores    | Synchronization         |
| Linux         | Operating environment   |

---

# 🚀 Running the Project

## Execute

```bash
bash bashrun.sh
```

---

# 🎛️ Runtime Flags

| Flag | Description              |
| ---- | ------------------------ |
| `-D` | Input directory          |
| `-O` | Output directory         |
| `-T` | Number of worker threads |
| `-S` | Queue size               |

---

# 💻 Example Usage

```bash
bash bashrun.sh -D input_data -O output_data -T 4 -S 10
```

---

# 🔄 IPC Design

| IPC Mechanism | Usage                              |
| ------------- | ---------------------------------- |
| Named Pipes   | Ingester → Processor communication |
| Shared Memory | Processor → Reporter communication |
| Semaphores    | Synchronization between modules    |
| Mutexes       | Safe thread access                 |

---

# 🔥 Parallel Processing Flow

```txt
CSV Input
    ↓
Ingester
    ↓
Named Pipe
    ↓
Processor Queue
    ↓
Worker Threads
    ↓
Aggregation Table
    ↓
Shared Memory
    ↓
Reporter
    ↓
CSV + TXT Output
```

---

# ⚠️ Challenges Faced

* 🔒 Race conditions during aggregation
* ⚡ Pipe buffer limitations
* 🧵 Thread synchronization complexity
* 🧹 Proper cleanup of IPC resources
* 📦 Efficient chunk distribution among threads

---


# 🌟 Operating System Concepts Demonstrated

* Process Management
* Thread Management
* Producer-Consumer Model
* Synchronization
* Shared Memory
* Inter-Process Communication
* Parallel Computing
* Resource Cleanup

---

# 📂 Output

The system generates:

* 📄 CSV Reports
* 📑 Human-readable TXT Reports

Stored in the configured output directory.

---

<div align="center">

## ⚡ Built for High Performance Linux Parallelism ⚡

</div>
