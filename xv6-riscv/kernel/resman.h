#ifndef _RESMAN_H_
#define _RESMAN_H_

#include "types.h"
#include "param.h"

#define MAX_RESOURCES 8
#define MAX_RES_INSTANCES 32

// Return codes for resource requests
#define RES_ALLOC_SUCCESS   0
#define RES_ALLOC_DELAYED   1
#define RES_ALLOC_REJECTED -1
#define RES_ALLOC_ERR      -2

// Prevention policies
#define POLICY_NONE         0  // Traditional unmanaged (allows deadlocks to form)
#define POLICY_BANKER       1  // Deterministic safety check only
#define POLICY_NEUROGUARD   2  // Hybrid AI predictive prevention

// User-facing structures
struct res_info {
  int id;
  int total;
  int available;
  int allocated_total;
};

struct res_status_user {
  int num_resources;
  struct res_info resources[MAX_RESOURCES];
  int active_procs;
  int allocation[NPROC][MAX_RESOURCES];
  int request[NPROC][MAX_RESOURCES];
  int pids[NPROC];
};

struct deadlock_status_user {
  int is_deadlocked;
  int num_deadlocked_procs;
  int deadlocked_pids[NPROC];
  int cycle_detected;
  int cycle_length;
  int cycle_pids[NPROC];
  int risk_score;       // 0 to 100
  int last_decision;    // 0: ALLOW, 1: DELAY, 2: REJECT
};

// Features extracted in C for inference
struct ml_features {
  int active_procs;          // Number of active processes
  int total_resources;       // Number of registered resources
  int available_ratio_pct;   // (Available / Total) * 100
  int allocated_ratio_pct;   // (Allocated / Total) * 100
  int pending_requests;      // Total pending requested instances
  int waiting_procs;         // Number of processes currently blocked
  int contention_ratio_pct;  // (Pending / (Available + 1)) * 100
  int max_chain_length;      // Longest dependency chain in WFG
  int has_cycle;             // 1 if cycle detected in WFG, 0 otherwise
};

// Kernel internal functions
void resman_init(void);
int  resman_register(int id, int total_instances);
int  resman_request(int pid, int id, int count);
int  resman_release(int pid, int id, int count);
void resman_on_exit(int pid);
int  resman_get_status(struct res_status_user *status);
int  resman_get_deadlock_status(struct deadlock_status_user *ds);
int  resman_get_risk_score(void);
int  resman_set_policy(int policy);

// ML Inference in C
int  neuroguard_predict_risk(const struct ml_features *f);

#endif
