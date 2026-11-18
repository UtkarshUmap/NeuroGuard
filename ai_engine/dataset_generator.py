#!/usr/bin/env python3
"""
NeuroGuard Synthetic Dataset Generator
Generates realistic multi-process, multi-resource allocation scenarios
with deterministic algorithmic ground-truth labeling.
"""

import numpy as np
import pandas as pd
import random

def is_deadlocked(avail, alloc, req):
    """Deterministic Multi-Instance Deadlock Detection (Silberschatz / Dijkstra)"""
    num_procs, num_res = alloc.shape
    work = avail.copy()
    finish = np.zeros(num_procs, dtype=bool)

    for i in range(num_procs):
        if np.all(alloc[i] == 0):
            finish[i] = True

    progress = True
    while progress:
        progress = False
        for i in range(num_procs):
            if not finish[i] and np.all(req[i] <= work):
                work += alloc[i]
                finish[i] = True
                progress = True

    # Any process with finish == False and non-zero alloc is deadlocked
    return np.any(~finish & (np.sum(alloc, axis=1) > 0))

def build_wfg_and_detect_cycles(alloc, req):
    """Construct Wait-For Graph and detect cycles / max chain length"""
    num_procs, num_res = alloc.shape
    adj = np.zeros((num_procs, num_procs), dtype=bool)

    for i in range(num_procs):
        for r in range(num_res):
            if req[i, r] > 0:
                for j in range(num_procs):
                    if i != j and alloc[j, r] > 0:
                        adj[i, j] = True

    # Cycle detection using DFS
    visited = [0] * num_procs
    rec_stack = [0] * num_procs
    has_cycle = False

    def dfs(u):
        nonlocal has_cycle
        visited[u] = 1
        rec_stack[u] = 1
        for v in range(num_procs):
            if adj[u, v]:
                if not visited[v]:
                    dfs(v)
                elif rec_stack[v]:
                    has_cycle = True
        rec_stack[u] = 0

    for i in range(num_procs):
        if not visited[i]:
            dfs(i)

    # Max chain length
    max_chain = 0
    for i in range(num_procs):
        chain = 0
        curr = i
        visited_nodes = {curr}
        for _ in range(num_procs):
            nxt_candidates = [v for v in range(num_procs) if adj[curr, v] and v not in visited_nodes]
            if not nxt_candidates:
                break
            curr = nxt_candidates[0]
            visited_nodes.add(curr)
            chain += 1
        max_chain = max(max_chain, chain)

    return int(has_cycle), max_chain

def generate_sample():
    num_procs = random.randint(2, 12)
    num_res = random.randint(2, 6)

    # Resource total capacities
    totals = np.array([random.randint(1, 8) for _ in range(num_res)])
    avail = totals.copy()

    alloc = np.zeros((num_procs, num_res), dtype=int)
    req = np.zeros((num_procs, num_res), dtype=int)

    # Contention type: 0: normal/safe, 1: high contention, 2: intentional circular lock
    scenario_type = random.choices([0, 1, 2], weights=[0.45, 0.35, 0.20])[0]

    if scenario_type == 0:
        # Safe, distributed
        for i in range(num_procs):
            for r in range(num_res):
                if avail[r] > 0 and random.random() < 0.4:
                    take = random.randint(1, min(2, avail[r]))
                    alloc[i, r] += take
                    avail[r] -= take
                elif random.random() < 0.2:
                    req[i, r] = random.randint(1, totals[r])

    elif scenario_type == 1:
        # High contention
        for i in range(num_procs):
            for r in range(num_res):
                if avail[r] > 0 and random.random() < 0.7:
                    take = random.randint(1, avail[r])
                    alloc[i, r] += take
                    avail[r] -= take
                if random.random() < 0.5:
                    req[i, r] = random.randint(1, totals[r])

    else:
        # Circular wait candidate
        for i in range(min(num_procs, 4)):
            r_held = i % num_res
            r_wanted = (i + 1) % num_res
            alloc[i, r_held] = 1
            avail[r_held] = max(0, avail[r_held] - 1)
            req[i, r_wanted] = 1

    # Extract features
    has_cycle, max_chain = build_wfg_and_detect_cycles(alloc, req)
    deadlocked = is_deadlocked(avail, alloc, req)

    tot_inst = int(np.sum(totals))
    avail_inst = int(np.sum(avail))
    alloc_inst = int(np.sum(alloc))
    pending_inst = int(np.sum(req))
    waiting_procs = int(np.sum(np.sum(req, axis=1) > 0))

    avail_ratio = int((avail_inst * 100) / tot_inst) if tot_inst > 0 else 0
    alloc_ratio = int((alloc_inst * 100) / tot_inst) if tot_inst > 0 else 0
    contention_ratio = int((pending_inst * 100) / (avail_inst + 1))

    # Label: 0 = Safe, 1 = Unsafe/Contended, 2 = Deadlocked
    if deadlocked:
        label = 2
    elif has_cycle or contention_ratio > 60 or avail_ratio < 20:
        label = 1
    else:
        label = 0

    return {
        'active_procs': num_procs,
        'total_resources': num_res,
        'available_ratio_pct': avail_ratio,
        'allocated_ratio_pct': alloc_ratio,
        'pending_requests': pending_inst,
        'waiting_procs': waiting_procs,
        'contention_ratio_pct': contention_ratio,
        'max_chain_length': max_chain,
        'has_cycle': has_cycle,
        'risk_label': label
    }

def main(n_samples=12000, output_path='ai_engine/datasets/deadlock_dataset.csv'):
    print(f"Generating {n_samples} synthetic resource allocation samples...")
    rows = [generate_sample() for _ in range(n_samples)]
    df = pd.DataFrame(rows)
    df.to_csv(output_path, index=False)
    print(f"Dataset generated and saved to {output_path}")
    print("Class distribution:")
    print(df['risk_label'].value_counts(normalize=True))

if __name__ == '__main__':
    main()
