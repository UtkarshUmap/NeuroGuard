# PROJECT: NEUROGUARD
## AI-Powered Predictive Deadlock Detection and Prevention in xv6 Operating System

Act as a senior Operating Systems engineer, xv6 kernel developer, Machine Learning engineer, and software architect.

Your task is to build a complete, functional, well-tested academic OS project named:

**NeuroGuard – AI-Powered Predictive Deadlock Detection and Prevention in xv6**

This is an Operating Systems course project. The implementation must demonstrate genuine modifications to the xv6 kernel, integration of machine learning, resource allocation monitoring, deadlock detection, and deadlock prevention.

This is NOT supposed to be merely a Python simulation or a dashboard.

The actual xv6 kernel must be modified to support resource management and deadlock monitoring.

---

# 1. FIRST: INSPECT THE EXISTING REPOSITORY

Before writing any code:

1. Inspect the complete repository structure.
2. Identify whether the repository uses xv6-riscv or another xv6 version.
3. Understand the existing kernel architecture.
4. Locate:
   - proc.c
   - proc.h
   - syscall.c
   - syscall.h
   - sysproc.c
   - user.h
   - usys.pl
   - Makefile
   - spinlock.c
   - relevant memory and process management files.
5. Understand how system calls are implemented.
6. Understand process creation, termination, sleeping, waking, and locking.
7. Identify the existing build and QEMU execution commands.

Do not assume the repository structure.

Do not overwrite existing xv6 implementations unnecessarily.

Preserve the original xv6 functionality.

Create a development plan and then implement it incrementally.

---

# 2. PROJECT OBJECTIVE

Traditional deadlock detection generally identifies deadlocks after they occur.

NeuroGuard aims to introduce an intelligent predictive resource management mechanism.

The system should:

1. Track resource allocation and resource requests.
2. Monitor process-resource dependencies.
3. Predict potential deadlock risks using Machine Learning.
4. Detect actual deadlocks using established OS algorithms.
5. Prevent unsafe resource allocations.
6. Explain why a resource request is considered dangerous.
7. Provide measurable performance comparisons.

The project must clearly distinguish:

- Safe state
- Unsafe state
- Potential deadlock
- Actual deadlock

Never treat an unsafe state as automatically equivalent to an actual deadlock.

---

# 3. XV6 KERNEL IMPLEMENTATION

Implement a resource management subsystem inside xv6.

## 3.1 Resource Manager

Create a resource manager that tracks:

- Resource IDs
- Total resource instances
- Available resource instances
- Resource allocation per process
- Pending resource requests
- Process ownership information

Use a fixed-size resource table initially, because xv6 is a small educational operating system.

Support multiple resource instances.

Define appropriate limits for:

- Maximum resources
- Maximum processes
- Maximum resource instances

Make these limits configurable.

Use appropriate kernel synchronization mechanisms to protect shared resource-management structures.

Avoid deadlocks inside the resource manager itself.

Pay special attention to lock ordering.

## 3.2 Resource Operations

Implement resource operations that xv6 user programs can invoke.

Required operations:

- Initialize/register resources.
- Request a resource.
- Release a resource.
- Query resource availability.
- Retrieve allocation information.

Use meaningful and consistent naming.

Suggested system calls:

- register_resource()
- request_resource()
- release_resource()
- get_resource_status()
- get_deadlock_status()
- get_risk_score()

Follow the actual xv6 system-call implementation conventions.

Validate user arguments carefully.

Handle invalid resource IDs, invalid quantities, process termination, and resource exhaustion.

## 3.3 Resource Allocation Tracking

Maintain allocation and request matrices.

For example:

Allocation Matrix:

         R1  R2  R3
P1        1   0   0
P2        0   1   0
P3        0   0   1

Request Matrix:

         R1  R2  R3
P1        0   1   0
P2        1   0   0
P3        0   0   0

Keep the matrices consistent with actual resource ownership.

Release all resources owned by a process when it exits.

Handle waiting processes and resource wakeups correctly.

Do not allow resource allocations to become inconsistent during concurrent requests.

---

# 4. DEADLOCK DETECTION ENGINE

Implement traditional OS deadlock detection.

Support the multiple-instance resource detection algorithm.

The algorithm should:

1. Copy the Available vector into Work.
2. Identify processes whose outstanding requests can be satisfied.
3. Simulate their completion and resource release.
4. Repeat until no additional process can finish.
5. Identify remaining processes as deadlocked candidates.

For single-instance resource scenarios, additionally implement a Wait-For Graph.

Detect cycles in the graph.

Provide:

- Deadlock status
- Involved process IDs
- Involved resource IDs
- Circular dependency information
- Detection timestamp or event sequence

Do not confuse deadlock detection with Banker's safety algorithm.

Implement appropriate tests for both single-instance and multiple-instance scenarios.

---

# 5. MACHINE LEARNING COMPONENT

This is a major component of the project.

Build a genuine machine learning pipeline using Python and Scikit-learn.

Do not simply hardcode AI predictions.

## 5.1 Synthetic Dataset Generation

Create a Python dataset generator.

Generate realistic resource allocation scenarios with varying:

- Number of processes
- Number of resources
- Available resource instances
- Allocated resources
- Pending requests
- Resource contention
- Process waiting patterns
- Dependency graph properties

Generate at least 10,000 samples initially, with configurable dataset size.

Include:

- Safe scenarios
- Unsafe scenarios
- Actual deadlock scenarios
- High-contention but non-deadlocked scenarios

Ensure that labels are generated using deterministic algorithmic ground truth.

Avoid data leakage.

Do not include the target label or direct copies of it as input features.

Use separate training, validation, and testing datasets.

Document the labeling methodology.

## 5.2 Feature Engineering

Extract meaningful features such as:

- Active process count
- Resource count
- Available resource ratio
- Allocated resource ratio
- Pending request count
- Waiting process count
- Resource contention ratio
- Dependency graph edge count
- Number of cycles
- Maximum dependency chain length
- Resource utilization
- Historical waiting statistics

Use only information genuinely available at prediction time.

## 5.3 Train Models

Train and compare:

1. Decision Tree Classifier
2. Random Forest Classifier

Evaluate using:

- Accuracy
- Precision
- Recall
- F1-score
- Confusion matrix
- ROC-AUC where applicable

Give special importance to recall for dangerous scenarios because false negatives can allow dangerous allocations.

Save trained models and preprocessing information.

Generate performance comparison graphs.

Document the best-performing model.

## 5.4 Prediction Engine

Build a prediction module that accepts current resource allocation information and returns:

- Predicted risk class
- Risk probability
- Relevant feature information
- Prediction explanation

Use explainable model outputs.

Do not present a probability as a guaranteed probability of actual deadlock.

Clearly distinguish predictive risk from algorithmically confirmed deadlock.

---

# 6. INTEGRATE AI WITH XV6

The ML component must meaningfully interact with xv6.

Do not stop after training a Python model.

Design a lightweight integration architecture.

Because running Python or Scikit-learn inside the xv6 kernel is inappropriate, use a deployable inference mechanism.

Preferred approach:

1. Train the model in Python.
2. Convert the trained Decision Tree or Random Forest into lightweight C inference logic.
3. Generate C decision rules or a compact model representation.
4. Integrate the inference implementation into the xv6 kernel or a tightly controlled kernel-side resource-management component.
5. Ensure that inference does not require an external Python runtime.

If full model integration into the kernel is impractical, implement a clearly documented alternative that transfers model predictions into xv6 through a controlled interface.

Do not fake AI integration.

The final xv6 system must demonstrate that its resource-management decisions use actual trained-model predictions.

Implement a hybrid decision mechanism:

AI prediction:
- Estimates potential deadlock risk.

Traditional detection:
- Verifies actual deadlock conditions.

Safety verification:
- Prevents confirmed unsafe allocations according to the selected policy.

An AI prediction alone must never be allowed to incorrectly terminate a process or arbitrarily deny resources.

Log all decisions.

---

# 7. DEADLOCK PREVENTION

Implement a prevention/intervention mechanism.

When a process requests a resource:

1. Collect the current resource state.
2. Extract model features.
3. Obtain the ML risk prediction.
4. Run deterministic safety verification.
5. Decide whether the request should be granted, delayed, or rejected.

Support at least these outcomes:

- ALLOW
- DELAY
- REJECT

Explain the reason for each decision.

Implement blocking and waking correctly if resource requests are delayed.

Prevent starvation where possible.

Use appropriate process wakeup mechanisms.

Clearly document the distinction between deadlock prevention and avoidance.

For multi-instance resources, use an appropriate safety-checking mechanism when enforcing safe-state allocation.

---

# 8. XV6 USER PROGRAMS AND DEMONSTRATIONS

Create multiple user-space programs inside xv6.

Required programs:

## Program 1: safe_test

Demonstrates a safe resource allocation scenario.

Expected:
- Requests granted.
- No deadlock.
- System continues execution.

## Program 2: deadlock_test

Creates a classic circular wait.

Example:

P1:
- Acquire R1.
- Request R2.

P2:
- Acquire R2.
- Request R1.

Expected:
- The system identifies the potential deadlock.
- The prevention mechanism delays or rejects a dangerous request.
- The system does not hang permanently.

Also provide a mode that allows a controlled deadlock to occur for testing the detection engine.

## Program 3: high_contention_test

Simulates multiple processes competing for limited resources.

Expected:
- Resource contention is measured.
- ML risk prediction is displayed.
- Safe requests continue.
- Dangerous requests are handled.

## Program 4: resource_stress_test

Tests resource tracking under multiple processes and repeated allocations/releases.

## Program 5: neuroguard_demo

A comprehensive demonstration combining all major features.

All programs must be included in the xv6 Makefile.

---

# 9. PERFORMANCE BENCHMARKING

Build a reproducible benchmarking framework.

Compare:

1. Traditional resource management without predictive intervention.
2. Traditional deadlock detection.
3. NeuroGuard hybrid AI-assisted resource management.

Measure:

- Number of deadlocks
- Number of prevented deadlocks
- False positive predictions
- False negative predictions
- Average resource allocation delay
- Average waiting time
- Throughput
- Detection overhead
- ML inference overhead

Run experiments across different workload sizes.

Generate actual results from executed experiments.

Never fabricate performance improvements or benchmark values.

If the AI mechanism performs worse for some workloads, document that honestly.

Produce CSV files and graphs.

---

# 10. VISUALIZATION DASHBOARD

Create a Python dashboard.

Prefer Streamlit if appropriate.

It should display:

- Active processes
- Available resources
- Allocated resources
- Pending requests
- Resource Allocation Graph
- Wait-For Graph
- Detected cycles
- AI-predicted risk
- Risk score
- Prevention decisions
- Model evaluation metrics
- Benchmark comparisons

Include a clear explanation panel:

"Why was this request delayed?"

Show the relevant resource dependencies and model features.

The dashboard should use actual experiment outputs, not fabricated real-time kernel information.

If live xv6 monitoring is unavailable, implement a log-export mechanism and clearly label the dashboard as an experiment-log visualizer.

---

# 11. PROJECT STRUCTURE

Organize the repository cleanly.

Suggested structure:

NeuroGuard/
|
|-- xv6/
|   |-- Original xv6 repository with kernel modifications
|   |-- Resource management implementation
|   |-- Deadlock detection implementation
|   |-- ML inference implementation
|   |-- User-space demonstration programs
|
|-- ai_engine/
|   |-- dataset_generator.py
|   |-- feature_extraction.py
|   |-- train.py
|   |-- evaluate.py
|   |-- inference.py
|   |-- model_export.py
|   |-- models/
|   |-- datasets/
|   |-- results/
|
|-- dashboard/
|   |-- app.py
|   |-- visualizations.py
|
|-- experiments/
|   |-- benchmark.py
|   |-- workload_generator.py
|   |-- results/
|
|-- tests/
|
|-- docs/
|   |-- architecture.md
|   |-- algorithms.md
|   |-- implementation.md
|   |-- testing.md
|
|-- README.md
|-- REPORT.md
|-- requirements.txt
|-- LICENSE

Adapt this structure to the actual repository. Do not duplicate the xv6 repository unnecessarily.

---

# 12. TESTING REQUIREMENTS

Test every component.

Required tests:

1. Basic resource allocation.
2. Resource release.
3. Invalid resource request.
4. Multiple processes requesting resources.
5. Safe allocation scenario.
6. Unsafe allocation scenario.
7. Single-instance circular wait.
8. Multiple-instance deadlock detection.
9. Process termination while holding resources.
10. Concurrent resource requests.
11. ML model inference.
12. ML and deterministic detection disagreement.
13. Prevention mechanism.
14. Stress testing.

Run xv6 using QEMU.

Build the kernel after every major modification.

Fix compilation errors, kernel panics, synchronization bugs, and incorrect resource accounting.

Do not claim a test passes unless it has actually been executed.

---

# 13. DOCUMENTATION AND FINAL SUBMISSION

Prepare a professional academic project report.

Include:

1. Abstract
2. Introduction
3. Problem Statement
4. Motivation
5. Existing System
6. Proposed System
7. Objectives
8. System Architecture
9. xv6 Kernel Modifications
10. Resource Management Design
11. Deadlock Detection Algorithms
12. Machine Learning Methodology
13. Dataset Generation
14. Feature Engineering
15. Model Training and Evaluation
16. AI-xv6 Integration
17. Deadlock Prevention
18. Experimental Setup
19. Results and Graphs
20. Performance Comparison
21. Limitations
22. Future Scope
23. Conclusion
24. References

Also create:

- Detailed README with installation instructions.
- Exact xv6 build and run commands.
- AI model training commands.
- Dataset generation commands.
- Dashboard execution commands.
- Demonstration instructions.
- System architecture diagrams.
- Screenshots of actual execution.
- Viva preparation document containing at least 30 technical questions and answers.

Include the GitHub repository link in the final report after it becomes available.

Do not invent a GitHub URL.

---

# 14. DEVELOPMENT RULES

Follow these rules throughout development:

1. Work incrementally.
2. Inspect before modifying.
3. Preserve original xv6 functionality.
4. Do not create fake or placeholder implementations.
5. Do not hardcode predictions.
6. Do not fabricate experimental results.
7. Do not claim successful compilation without running the build.
8. Use meaningful function names and comments.
9. Follow xv6 coding conventions.
10. Explain important kernel modifications.
11. Avoid unnecessary dependencies.
12. Handle concurrency carefully.
13. Keep the system reproducible.
14. Ensure the final project can be built from a clean checkout.

After each phase:
- Summarize implemented features.
- List modified files.
- Run relevant tests.
- Report errors.
- Fix errors before proceeding.

Do not stop after creating a plan or generating the initial files.

Continue implementation until the project is complete.

If a feature cannot be implemented reliably, explain the limitation and provide the most technically sound alternative instead of pretending it works.

---

# 15. FINAL ACCEPTANCE CRITERIA

The project is considered complete only when:

- xv6 builds successfully.
- xv6 boots successfully in QEMU.
- Custom resource-management system calls work.
- Multiple processes can request and release resources.
- Resource allocation information remains consistent.
- Traditional deadlock detection works.
- Actual deadlock scenarios can be demonstrated.
- Synthetic dataset generation works.
- ML models train and evaluate successfully.
- Trained model inference is integrated into the xv6 resource-management workflow.
- AI-assisted decisions are explainable.
- Prevention mechanisms work.
- Benchmark results are reproducible.
- Dashboard displays actual experiment results.
- Documentation is complete.
- A clean project ZIP can be generated.

Start by inspecting the existing workspace.

Then create a phased implementation plan and execute it.

Your priority is a technically correct, genuinely integrated, demonstrable Operating Systems project, not merely a large amount of generated code.