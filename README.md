# NeuroGuard 🛡️
## AI-Powered Predictive Deadlock Detection and Prevention in the xv6 Operating System

NeuroGuard is an academic Operating Systems project that modifies the **xv6-riscv** kernel to support multi-instance resource management, graph-theoretic deadlock detection, and real-time AI-assisted predictive deadlock prevention.

---

## 📂 Repository Structure

```
OS_Project/
├── xv6-riscv/                    # Modified xv6 kernel & user programs
│   ├── kernel/
│   │   ├── resman.h              # Resource manager headers & data structures
│   │   ├── resman.c              # Core resource tracking, WFG & Work/Finish algorithms
│   │   ├── neuroguard_infer.c    # Transpiled C ML decision logic (zero-dependency)
│   │   ├── sysproc.c             # System call handlers (syscalls 24-30)
│   │   └── proc.c                # Process exit hook (auto-reclamation)
│   ├── user/
│   │   ├── safe_test.c           # Safe multi-resource allocation test
│   │   ├── deadlock_test.c       # Circular wait test (Detection & Prevention modes)
│   │   ├── contention.c          # High contention competition & dynamic risk test
│   │   ├── stress_test.c         # Process exit auto-cleanup & stress stability test
│   │   └── neuroguard.c          # Unified demonstration test runner
│   └── Makefile                  # xv6 build configuration with NeuroGuard targets
│
├── ai_engine/                    # Machine Learning pipeline (Python & Scikit-learn)
│   ├── dataset_generator.py      # Synthesizes 12,000+ realistic allocation states
│   ├── train.py                  # Trains Decision Tree & Random Forest models
│   ├── evaluate.py               # Generates confusion matrices & feature plots
│   ├── model_export.py           # Transpiles Decision Tree directly into kernel C
│   ├── models/                   # Saved .joblib model files
│   ├── datasets/                 # deadlock_dataset.csv
│   └── results/                  # Metrics JSON and PNG evaluation charts
│
├── experiments/                  # Reproducible Benchmarking Framework
│   ├── benchmark.py              # Comparative performance benchmarks
│   └── results/                  # Benchmark CSV data and tradeoff graphs
│
├── dashboard/                    # Interactive Streamlit Web Dashboard
│   └── app.py                    # RAG/WFG visualizer & explainability panel
│
├── REPORT.pdf                    # Humanized submission report with figures and terminal evidence
├── REPORT.docx                   # Editable report source
├── REPORT.md                     # Report notes and submission reminder
└── README.md                     # This documentation file
```

Open [REPORT.pdf](REPORT.pdf) for the formatted project report. The report links to the team's repository: https://github.com/UtkarshUmap/NeuroGuard.

---

## 🚀 Quick Start Guide

### 1. Build and Run xv6 Kernel with NeuroGuard
Ensure `riscv64-unknown-elf-gcc` and `qemu-system-riscv64` are installed:
```bash
cd xv6-riscv
make clean
make -j$(nproc)
make qemu
```
To exit QEMU at any time, press `Ctrl+A` then `X`.

---

## 🧪 Running Kernel Demonstrations Inside xv6

Once xv6 boots to the shell prompt (`$ `):

### 1. Safe Allocation Test
Demonstrates concurrent, safe resource allocations without circular wait:
```bash
$ safe_test
```

### 2. Classic Deadlock Formation & Detection (Mode 0)
Runs with AI prevention disabled (`POLICY_NONE`). Causes an unmanaged circular lock between two processes ($P_1 \leftrightarrow P_2$). The detection engine flags the deadlocked PIDs and Wait-For Graph cycle, then terminates a victim to recover:
```bash
$ deadlock_test 0
```

### 3. NeuroGuard AI Predictive Prevention (Mode 1)
Runs with AI prevention active (`POLICY_NEUROGUARD`). When the hazardous request is made, NeuroGuard detects the circular dependency, intercepts the request, and rejects it with `RES_ALLOC_REJECTED`. The process backs off and retries, and both processes finish safely:
```bash
$ deadlock_test 1
```

### 4. High Resource Contention Test
Simulates 4 concurrent processes competing for scarce resources with dynamic ML risk evaluation:
```bash
$ contention
```

### 5. Resource Stress & Auto-Cleanup Test
Verifies that resources held by terminated or killed processes are 100% reclaimed by the kernel without memory or resource leaks:
```bash
$ stress_test
```

### 6. Full Demonstration Suite
Executes the entire test suite in sequence:
```bash
$ neuroguard
```

---

## 🧠 Retraining the Machine Learning Pipeline

To regenerate the synthetic dataset, retrain models, produce evaluation charts, and re-export the kernel C inference logic:
```bash
# 1. Generate 12,000 synthetic states with deterministic ground truth
python3 ai_engine/dataset_generator.py

# 2. Train Decision Tree and Random Forest classifiers
python3 ai_engine/train.py

# 3. Generate confusion matrices and feature importance graphs
python3 ai_engine/evaluate.py

# 4. Transpile the trained Decision Tree into xv6 kernel C code
python3 ai_engine/model_export.py

# 5. Recompile the kernel with the updated model
cd xv6-riscv && make -j$(nproc) && cd ..
```

---

## 📊 Running the Benchmarks
To run comparative experiments across workload sizes:
```bash
python3 experiments/benchmark.py
```
Output results and graphs are saved in `experiments/results/`.

---

## 💻 Launching the Streamlit Dashboard
To launch the interactive web visualizer:
```bash
streamlit run dashboard/app.py
```
View the dashboard at `http://localhost:8501`.

---

