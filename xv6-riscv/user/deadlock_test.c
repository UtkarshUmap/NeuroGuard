#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/resman.h"

int
main(int argc, char *argv[])
{
  int mode = 1; // Default: 1 = AI Prevention ON, 0 = Detection Test (Prevention OFF)
  if (argc > 1 && argv[1][0] == '0') {
    mode = 0;
  }

  printf("\n=== [NeuroGuard] Deadlock Test ===\n");
  printf("Mode: %s\n", mode == 0 ? "0 (Prevention OFF - Classic Deadlock Formation)"
                                 : "1 (AI Prevention ON - Hazard Intervention)");

  // Register single-instance resources R1 and R2
  res_register(1, 1);
  res_register(2, 1);

  if (mode == 0) {
    res_set_policy(POLICY_NONE);
  } else {
    res_set_policy(POLICY_NEUROGUARD);
  }

  int worker_pid = fork();
  if (worker_pid < 0) {
    printf("Fork failed!\n");
    exit(1);
  }

  // Fork a monitor process that checks for deadlock after processes initialize
  int mon_pid = fork();
  if (mon_pid == 0) {
    pause(15); // Wait for worker processes to acquire and request
    static struct deadlock_status_user ds;
    res_deadlock_status(&ds);

    printf("\n--- Monitor Report ---\n");
    printf("Is Deadlocked: %s\n", ds.is_deadlocked ? "YES (CONFIRMED)" : "NO");
    printf("Cycle Detected: %s (Length: %d)\n", ds.cycle_detected ? "YES" : "NO", ds.cycle_length);
    printf("ML Risk Score: %d%%\n", ds.risk_score);
    if (ds.cycle_detected) {
      printf("Cycle PIDs: ");
      for (int i = 0; i < ds.cycle_length; i++) {
        printf("%d ", ds.cycle_pids[i]);
      }
      printf("\n");
    }

    if (mode == 0 && ds.is_deadlocked) {
      printf("[Monitor] Deadlock successfully demonstrated! Terminating victim PID %d to break deadlock...\n", worker_pid);
      kill(worker_pid);
    } else if (mode == 1) {
      printf("[Monitor] Prevention active: Deadlock successfully avoided!\n");
    }
    printf("----------------------\n\n");
    exit(0);
  }

  if (worker_pid == 0) {
    // Worker 1: Acquire R1, then request R2
    int my_pid = getpid();
    printf("[Worker 1 (PID %d)] Acquiring R1...\n", my_pid);
    res_request(1, 1);
    printf("[Worker 1 (PID %d)] Holding R1. Pausing...\n", my_pid);
    pause(10);

    printf("[Worker 1 (PID %d)] Requesting R2...\n", my_pid);
    int ret = res_request(2, 1);
    if (ret == RES_ALLOC_REJECTED) {
      printf("[Worker 1 (PID %d)] Request REJECTED by NeuroGuard to prevent circular deadlock!\n", my_pid);
      printf("[Worker 1 (PID %d)] Backing off: releasing R1 temporarily...\n", my_pid);
      res_release(1, 1);
      pause(20);
      printf("[Worker 1 (PID %d)] Retrying R1 and R2...\n", my_pid);
      res_request(1, 1);
      res_request(2, 1);
    }
    printf("[Worker 1 (PID %d)] Finished with both resources. Releasing...\n", my_pid);
    res_release(2, 1);
    res_release(1, 1);
    exit(0);
  } else {
    // Parent / Worker 2: Acquire R2, then request R1
    int my_pid = getpid();
    pause(4); // Let child acquire R1 first
    printf("[Worker 2 (PID %d)] Acquiring R2...\n", my_pid);
    res_request(2, 1);
    printf("[Worker 2 (PID %d)] Holding R2. Pausing...\n", my_pid);
    pause(10);

    printf("[Worker 2 (PID %d)] Requesting R1...\n", my_pid);
    int ret = res_request(1, 1);
    if (ret == RES_ALLOC_REJECTED) {
      printf("[Worker 2 (PID %d)] Request REJECTED by NeuroGuard to prevent circular deadlock!\n", my_pid);
      printf("[Worker 2 (PID %d)] Backing off: releasing R2 temporarily...\n", my_pid);
      res_release(2, 1);
      pause(20);
      printf("[Worker 2 (PID %d)] Retrying R2 and R1...\n", my_pid);
      res_request(2, 1);
      res_request(1, 1);
    }
    printf("[Worker 2 (PID %d)] Finished with both resources. Releasing...\n", my_pid);
    res_release(1, 1);
    res_release(2, 1);

    wait(0); // Wait for worker 1
    wait(0); // Wait for monitor

    printf("=== Deadlock Test Finished ===\n\n");
    exit(0);
  }
}
