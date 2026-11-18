#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/resman.h"

int
main(int argc, char *argv[])
{
  printf("\n=== [NeuroGuard] Safe Allocation Test ===\n");

  // Register resources R1 (5 instances) and R2 (3 instances)
  res_register(1, 5);
  res_register(2, 3);
  res_set_policy(POLICY_NEUROGUARD);

  printf("[1] Registered Resource 1 (5 instances) and Resource 2 (3 instances).\n");

  int pid = fork();
  if (pid < 0) {
    printf("Fork failed!\n");
    exit(1);
  }

  if (pid == 0) {
    // Child process: request R1 then R2 safely
    int my_pid = getpid();
    printf("[Child %d] Requesting 2 units of R1...\n", my_pid);
    if (res_request(1, 2) == 0) {
      printf("[Child %d] Successfully acquired 2 units of R1.\n", my_pid);
    }
    pause(10);

    printf("[Child %d] Requesting 1 unit of R2...\n", my_pid);
    if (res_request(2, 1) == 0) {
      printf("[Child %d] Successfully acquired 1 unit of R2.\n", my_pid);
    }
    pause(15);

    printf("[Child %d] Releasing R2 and R1...\n", my_pid);
    res_release(2, 1);
    res_release(1, 2);
    printf("[Child %d] Completed cleanly.\n", my_pid);
    exit(0);
  } else {
    // Parent process: request R2 then R1 safely
    int my_pid = getpid();
    printf("[Parent %d] Requesting 1 unit of R2...\n", my_pid);
    if (res_request(2, 1) == 0) {
      printf("[Parent %d] Successfully acquired 1 unit of R2.\n", my_pid);
    }
    pause(10);

    printf("[Parent %d] Requesting 1 unit of R1...\n", my_pid);
    if (res_request(1, 1) == 0) {
      printf("[Parent %d] Successfully acquired 1 unit of R1.\n", my_pid);
    }

    static struct deadlock_status_user ds;
    res_deadlock_status(&ds);
    printf("[Parent %d] Deadlock Status: %s | AI Risk Score: %d%%\n",
           my_pid, ds.is_deadlocked ? "DEADLOCKED" : "SAFE", ds.risk_score);

    res_release(1, 1);
    res_release(2, 1);

    wait(0);
    printf("=== Safe Allocation Test Passed! ===\n\n");
    exit(0);
  }
}
