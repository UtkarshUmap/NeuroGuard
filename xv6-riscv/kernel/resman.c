#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "resman.h"

//
// NeuroGuard Kernel Resource Manager
// Tracks multi-instance resources, process allocations, pending requests,
// runs deterministic deadlock detection, and applies AI predictive prevention.
//

static struct spinlock resman_lock;
static int res_channel; // Wait channel for blocking requests

static struct res_entry {
  int id;
  int total;
  int available;
  int registered;
} resources[MAX_RESOURCES];

static int num_registered = 0;

// Per-process tracking tables indexed by slot [0..NPROC-1]
static int proc_pids[NPROC];
static int allocation[NPROC][MAX_RESOURCES];
static int request[NPROC][MAX_RESOURCES];

// Static WFG adjacency matrix: bit j in adj[i] means proc slot i waits for proc slot j
static uint64 adj[NPROC];
static int static_deadlocked_pids[NPROC];
static int static_cycle_pids[NPROC];
static int static_path[NPROC];

static int current_policy = POLICY_NEUROGUARD;
static int last_decision = 0; // 0: ALLOW, 1: DELAY, 2: REJECT
static int last_risk_score = 0;

// Find or allocate a slot for a process PID
static int
get_slot(int pid)
{
  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == pid)
      return i;
  }
  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == 0) {
      proc_pids[i] = pid;
      for (int r = 0; r < MAX_RESOURCES; r++) {
        allocation[i][r] = 0;
        request[i][r] = 0;
      }
      return i;
    }
  }
  return -1;
}

// Find resource index by resource ID
static int
get_res_index(int id)
{
  for (int i = 0; i < MAX_RESOURCES; i++) {
    if (resources[i].registered && resources[i].id == id)
      return i;
  }
  return -1;
}

void
resman_init(void)
{
  initlock(&resman_lock, "resman");
  acquire(&resman_lock);

  num_registered = 0;
  for (int i = 0; i < MAX_RESOURCES; i++) {
    resources[i].registered = 0;
    resources[i].id = 0;
    resources[i].total = 0;
    resources[i].available = 0;
  }
  for (int i = 0; i < NPROC; i++) {
    proc_pids[i] = 0;
    adj[i] = 0;
    for (int r = 0; r < MAX_RESOURCES; r++) {
      allocation[i][r] = 0;
      request[i][r] = 0;
    }
  }
  current_policy = POLICY_NEUROGUARD;
  last_decision = 0;
  last_risk_score = 0;

  release(&resman_lock);
  printk("NeuroGuard: Resource Manager initialized with AI Prevention active.\n");
}

int
resman_register(int id, int total_instances)
{
  if (id <= 0 || total_instances <= 0 || total_instances > MAX_RES_INSTANCES)
    return -1;

  acquire(&resman_lock);

  // Check if already registered
  for (int i = 0; i < MAX_RESOURCES; i++) {
    if (resources[i].registered && resources[i].id == id) {
      release(&resman_lock);
      return 0; // Already registered
    }
  }

  // Find free resource slot
  for (int i = 0; i < MAX_RESOURCES; i++) {
    if (!resources[i].registered) {
      resources[i].registered = 1;
      resources[i].id = id;
      resources[i].total = total_instances;
      resources[i].available = total_instances;
      num_registered++;
      release(&resman_lock);
      return 0;
    }
  }

  release(&resman_lock);
  return -1; // Resource table full
}

//
// Wait-For Graph (WFG) Construction and Cycle Detection (Bitmask Based)
//
static int
check_cycle_dfs(int u, uint64 *visited, uint64 *rec_stack, int path[NPROC],
                int depth, int *cycle_len, int cycle_out[NPROC])
{
  *visited |= (1ULL << u);
  *rec_stack |= (1ULL << u);
  path[depth] = u;

  for (int v = 0; v < NPROC; v++) {
    if (adj[u] & (1ULL << v)) {
      if (!(*visited & (1ULL << v))) {
        if (check_cycle_dfs(v, visited, rec_stack, path, depth + 1, cycle_len, cycle_out))
          return 1;
      } else if (*rec_stack & (1ULL << v)) {
        // Cycle detected
        int start = 0;
        while (start <= depth && path[start] != v)
          start++;
        *cycle_len = 0;
        for (int k = start; k <= depth; k++) {
          cycle_out[(*cycle_len)++] = proc_pids[path[k]];
        }
        return 1;
      }
    }
  }

  *rec_stack &= ~(1ULL << u);
  return 0;
}

static int
detect_wfg_cycle(int *cycle_len, int cycle_pids[NPROC], int *max_chain)
{
  for (int i = 0; i < NPROC; i++)
    adj[i] = 0;

  // Build adjacency matrix: P_i waits for P_j if P_i requests R and P_j holds R
  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == 0) continue;
    for (int r = 0; r < MAX_RESOURCES; r++) {
      if (request[i][r] > 0) {
        for (int j = 0; j < NPROC; j++) {
          if (i != j && proc_pids[j] != 0 && allocation[j][r] > 0) {
            adj[i] |= (1ULL << j);
          }
        }
      }
    }
  }

  // Calculate maximum chain length
  int longest = 0;
  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == 0) continue;
    int has_incoming = 0;
    for (int k = 0; k < NPROC; k++) {
      if (adj[k] & (1ULL << i)) { has_incoming = 1; break; }
    }
    if (!has_incoming) {
      int curr = i;
      int chain = 0;
      for (int step = 0; step < NPROC; step++) {
        int next = -1;
        for (int nxt = 0; nxt < NPROC; nxt++) {
          if (adj[curr] & (1ULL << nxt)) { next = nxt; break; }
        }
        if (next == -1) break;
        chain++;
        curr = next;
      }
      if (chain > longest) longest = chain;
    }
  }
  *max_chain = longest;

  uint64 visited = 0;
  uint64 rec_stack = 0;
  *cycle_len = 0;

  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] != 0 && !(visited & (1ULL << i))) {
      if (check_cycle_dfs(i, &visited, &rec_stack, static_path, 0, cycle_len, cycle_pids))
        return 1;
    }
  }
  return 0;
}

//
// Multi-instance Deadlock Detection Algorithm
//
static int
detect_deadlock_internal(int deadlocked_pids[NPROC])
{
  int work[MAX_RESOURCES];
  uint64 finish = 0; // bit i set means proc slot i finished
  int num_deadlocked = 0;

  for (int r = 0; r < MAX_RESOURCES; r++) {
    work[r] = resources[r].registered ? resources[r].available : 0;
  }

  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == 0) {
      finish |= (1ULL << i);
    } else {
      int has_alloc = 0;
      for (int r = 0; r < MAX_RESOURCES; r++) {
        if (allocation[i][r] > 0) {
          has_alloc = 1;
          break;
        }
      }
      if (!has_alloc) finish |= (1ULL << i);
    }
  }

  int progress = 1;
  while (progress) {
    progress = 0;
    for (int i = 0; i < NPROC; i++) {
      if (proc_pids[i] != 0 && !(finish & (1ULL << i))) {
        int can_satisfy = 1;
        for (int r = 0; r < MAX_RESOURCES; r++) {
          if (request[i][r] > work[r]) {
            can_satisfy = 0;
            break;
          }
        }
        if (can_satisfy) {
          for (int r = 0; r < MAX_RESOURCES; r++) {
            work[r] += allocation[i][r];
          }
          finish |= (1ULL << i);
          progress = 1;
        }
      }
    }
  }

  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] != 0 && !(finish & (1ULL << i))) {
      deadlocked_pids[num_deadlocked++] = proc_pids[i];
    }
  }
  return num_deadlocked;
}

// Extract runtime ML features
static void
extract_features(struct ml_features *f)
{
  int total_res_inst = 0;
  int avail_res_inst = 0;
  int alloc_res_inst = 0;
  int pending_inst = 0;
  int waiting_cnt = 0;
  int active_cnt = 0;

  for (int r = 0; r < MAX_RESOURCES; r++) {
    if (resources[r].registered) {
      total_res_inst += resources[r].total;
      avail_res_inst += resources[r].available;
    }
  }

  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] != 0) {
      active_cnt++;
      int is_waiting = 0;
      for (int r = 0; r < MAX_RESOURCES; r++) {
        alloc_res_inst += allocation[i][r];
        if (request[i][r] > 0) {
          pending_inst += request[i][r];
          is_waiting = 1;
        }
      }
      if (is_waiting) waiting_cnt++;
    }
  }

  int cycle_len = 0;
  int max_chain = 0;
  int cycle = detect_wfg_cycle(&cycle_len, static_cycle_pids, &max_chain);

  f->active_procs = active_cnt;
  f->total_resources = num_registered;
  f->available_ratio_pct = total_res_inst > 0 ? (avail_res_inst * 100) / total_res_inst : 0;
  f->allocated_ratio_pct = total_res_inst > 0 ? (alloc_res_inst * 100) / total_res_inst : 0;
  f->pending_requests = pending_inst;
  f->waiting_procs = waiting_cnt;
  f->contention_ratio_pct = (avail_res_inst + 1 > 0) ? (pending_inst * 100) / (avail_res_inst + 1) : 0;
  f->max_chain_length = max_chain;
  f->has_cycle = cycle;
}

int
resman_request(int pid, int id, int count)
{
  if (pid <= 0 || id <= 0 || count <= 0)
    return RES_ALLOC_ERR;

  acquire(&resman_lock);

  int r_idx = get_res_index(id);
  if (r_idx < 0 || count > resources[r_idx].total) {
    release(&resman_lock);
    return RES_ALLOC_ERR;
  }

  int slot = get_slot(pid);
  if (slot < 0) {
    release(&resman_lock);
    return RES_ALLOC_ERR;
  }

  for (;;) {
    //
    // POLICY_NONE: Traditional unmanaged resource requests (allows deadlock to occur)
    //
    if (current_policy == POLICY_NONE) {
      if (resources[r_idx].available >= count) {
        resources[r_idx].available -= count;
        allocation[slot][r_idx] += count;
        request[slot][r_idx] = 0;
        last_decision = 0; // ALLOW
        release(&resman_lock);
        return RES_ALLOC_SUCCESS;
      } else {
        request[slot][r_idx] = count;
        last_decision = 1; // DELAY
        sleep_prepare(&res_channel);
        release(&resman_lock);
        sleep();
        acquire(&resman_lock);
        if (killed(myproc())) {
          request[slot][r_idx] = 0;
          release(&resman_lock);
          return -1;
        }
        continue;
      }
    }

    //
    // POLICY_NEUROGUARD & POLICY_BANKER: AI Predictive Prevention
    //
    struct ml_features feat;
    extract_features(&feat);
    last_risk_score = neuroguard_predict_risk(&feat);

    if (resources[r_idx].available >= count) {
      // Simulate grant
      resources[r_idx].available -= count;
      allocation[slot][r_idx] += count;

      struct ml_features post_feat;
      extract_features(&post_feat);
      int post_risk = neuroguard_predict_risk(&post_feat);

      if (current_policy == POLICY_NEUROGUARD && post_feat.has_cycle && post_risk > 80) {
        // Revert simulation: granting this creates an immediate deadlock cycle!
        resources[r_idx].available += count;
        allocation[slot][r_idx] -= count;

        request[slot][r_idx] = count;
        last_decision = 1; // DELAY dangerous request
        printk("NeuroGuard: [PREVENT] Dangerous request by PID %d for Res %d delayed (Risk: %d%%, WFG cycle detected)\n",
               pid, id, post_risk);

        sleep_prepare(&res_channel);
        release(&resman_lock);
        sleep();
        acquire(&resman_lock);
        if (killed(myproc())) {
          request[slot][r_idx] = 0;
          release(&resman_lock);
          return -1;
        }
        continue;
      }

      // Safe to grant
      request[slot][r_idx] = 0;
      last_decision = 0; // ALLOW
      release(&resman_lock);
      return RES_ALLOC_SUCCESS;
    } else {
      // Resource unavailable, process wants to wait
      request[slot][r_idx] = count;

      // In POLICY_NEUROGUARD, check if waiting forms an immediate fatal circular wait cycle
      int cycle_len = 0;
      int max_chain = 0;
      if (current_policy == POLICY_NEUROGUARD && detect_wfg_cycle(&cycle_len, static_cycle_pids, &max_chain)) {
        // Clear request to prevent circular wait from freezing the system
        request[slot][r_idx] = 0;
        last_decision = 2; // REJECT
        printk("NeuroGuard: [PREVENT] Request by PID %d for Res %d REJECTED (circular wait avoided!)\n",
               pid, id);
        release(&resman_lock);
        return RES_ALLOC_REJECTED;
      }

      last_decision = 1; // DELAY
      sleep_prepare(&res_channel);
      release(&resman_lock);
      sleep();
      acquire(&resman_lock);
      if (killed(myproc())) {
        request[slot][r_idx] = 0;
        release(&resman_lock);
        return -1;
      }
    }
  }
}

int
resman_release(int pid, int id, int count)
{
  if (pid <= 0 || id <= 0 || count <= 0)
    return -1;

  acquire(&resman_lock);

  int r_idx = get_res_index(id);
  if (r_idx < 0) {
    release(&resman_lock);
    return -1;
  }

  int slot = get_slot(pid);
  if (slot < 0 || allocation[slot][r_idx] < count) {
    release(&resman_lock);
    return -1;
  }

  allocation[slot][r_idx] -= count;
  resources[r_idx].available += count;

  // Wake up waiting processes
  wakeup(&res_channel);

  release(&resman_lock);
  return 0;
}

void
resman_on_exit(int pid)
{
  acquire(&resman_lock);

  for (int i = 0; i < NPROC; i++) {
    if (proc_pids[i] == pid) {
      for (int r = 0; r < MAX_RESOURCES; r++) {
        if (allocation[i][r] > 0) {
          resources[r].available += allocation[i][r];
          allocation[i][r] = 0;
        }
        request[i][r] = 0;
      }
      proc_pids[i] = 0; // Free slot
      wakeup(&res_channel);
      break;
    }
  }

  release(&resman_lock);
}

int
resman_get_status(struct res_status_user *status)
{
  acquire(&resman_lock);

  status->num_resources = num_registered;
  for (int r = 0; r < MAX_RESOURCES; r++) {
    status->resources[r].id = resources[r].id;
    status->resources[r].total = resources[r].total;
    status->resources[r].available = resources[r].available;
    status->resources[r].allocated_total = resources[r].total - resources[r].available;
  }

  status->active_procs = 0;
  for (int i = 0; i < NPROC; i++) {
    status->pids[i] = proc_pids[i];
    if (proc_pids[i] != 0)
      status->active_procs++;

    for (int r = 0; r < MAX_RESOURCES; r++) {
      status->allocation[i][r] = allocation[i][r];
      status->request[i][r] = request[i][r];
    }
  }

  release(&resman_lock);
  return 0;
}

int
resman_get_deadlock_status(struct deadlock_status_user *ds)
{
  acquire(&resman_lock);

  int num_dead = detect_deadlock_internal(static_deadlocked_pids);

  ds->is_deadlocked = (num_dead > 0) ? 1 : 0;
  ds->num_deadlocked_procs = num_dead;
  for (int i = 0; i < num_dead; i++) {
    ds->deadlocked_pids[i] = static_deadlocked_pids[i];
  }

  int cycle_len = 0;
  int max_chain = 0;
  int has_cycle = detect_wfg_cycle(&cycle_len, static_cycle_pids, &max_chain);

  ds->cycle_detected = has_cycle;
  ds->cycle_length = cycle_len;
  for (int i = 0; i < cycle_len; i++) {
    ds->cycle_pids[i] = static_cycle_pids[i];
  }

  struct ml_features feat;
  extract_features(&feat);
  last_risk_score = neuroguard_predict_risk(&feat);

  ds->risk_score = last_risk_score;
  ds->last_decision = last_decision;

  release(&resman_lock);
  return 0;
}

int
resman_get_risk_score(void)
{
  acquire(&resman_lock);
  struct ml_features feat;
  extract_features(&feat);
  int score = neuroguard_predict_risk(&feat);
  last_risk_score = score;
  release(&resman_lock);
  return score;
}

int
resman_set_policy(int policy)
{
  acquire(&resman_lock);
  current_policy = policy;
  release(&resman_lock);
  return 0;
}
