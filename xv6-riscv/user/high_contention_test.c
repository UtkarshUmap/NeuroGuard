#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/resman.h"

int
main(int argc, char *argv[])
{
  printf("\n=== [NeuroGuard] High Contention Test ===\n");
  printf("Simulating 4 concurrent processes competing for limited resources...\n");

  res_register(1, 2); // 2 instances of Resource 1
  res_register(2, 2); // 2 instances of Resource 2
  res_set_policy(POLICY_NEUROGUARD);

  int num_procs = 4;
  for (int i = 0; i < num_procs; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("Fork failed for worker %d\n", i);
      exit(1);
    }
    if (pid == 0) {
      int my_pid = getpid();
      for (int iter = 0; iter < 2; iter++) {
        int r_first = (i % 2 == 0) ? 1 : 2;
        int r_second = (i % 2 == 0) ? 2 : 1;

        printf("[Worker %d (PID %d)] Requesting R%d (iter %d)...\n", i, my_pid, r_first, iter);
        res_request(r_first, 1);

        int risk = res_risk_score();
        printf("[Worker %d (PID %d)] Acquired R%d. Current Risk: %d%%\n", i, my_pid, r_first, risk);
        pause(5);

        printf("[Worker %d (PID %d)] Requesting R%d...\n", i, my_pid, r_second);
        res_request(r_second, 1);
        pause(5);

        printf("[Worker %d (PID %d)] Releasing R%d and R%d\n", i, my_pid, r_second, r_first);
        res_release(r_second, 1);
        res_release(r_first, 1);
      }
      printf("[Worker %d (PID %d)] All iterations complete.\n", i, my_pid);
      exit(0);
    }
  }

  // Parent monitors and waits for all children
  for (int i = 0; i < num_procs; i++) {
    wait(0);
  }

  static struct deadlock_status_user ds;
  res_deadlock_status(&ds);
  printf("\n--- Final Status ---\n");
  printf("Active Deadlocks: %d\n", ds.num_deadlocked_procs);
  printf("Final Risk Score: %d%%\n", ds.risk_score);
  printf("=== High Contention Test Passed! ===\n\n");
  exit(0);
}
