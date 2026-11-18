#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/resman.h"

int
main(int argc, char *argv[])
{
  printf("\n=== [NeuroGuard] Resource Stress & Auto-Cleanup Test ===\n");

  res_register(1, 10);
  res_register(2, 10);
  res_register(3, 10);
  res_set_policy(POLICY_NEUROGUARD);

  printf("[Stress] Registered R1(10), R2(10), R3(10).\n");

  // Test 1: Process exits while holding resources (auto-cleanup verification)
  printf("[Stress 1] Testing automatic kernel cleanup upon process exit...\n");
  int pid = fork();
  if (pid == 0) {
    res_request(1, 4);
    res_request(2, 5);
    printf("[Child PID %d] Acquired 4 of R1 and 5 of R2. Exiting immediately WITHOUT releasing...\n", getpid());
    exit(0);
  }
  wait(0);

  static struct res_status_user status;
  res_status(&status);
  printf("[Parent] Checking resource recovery after child exit:\n");
  for (int r = 0; r < status.num_resources; r++) {
    printf("  Resource %d: Total=%d, Available=%d\n",
           status.resources[r].id, status.resources[r].total, status.resources[r].available);
    if (status.resources[r].total != status.resources[r].available) {
      printf("ERROR: Resource %d leaked!\n", status.resources[r].id);
      exit(1);
    }
  }
  printf("[Stress 1] SUCCESS: All resources automatically reclaimed by kernel!\n");

  // Test 2: Multi-process stress cycles
  printf("\n[Stress 2] Running 6 concurrent worker processes...\n");
  for (int i = 0; i < 6; i++) {
    int child = fork();
    if (child == 0) {
      for (int k = 0; k < 5; k++) {
        res_request(1, 1);
        res_request(2, 1);
        pause(2);
        res_release(1, 1);
        res_release(2, 1);
      }
      exit(0);
    }
  }

  for (int i = 0; i < 6; i++) {
    wait(0);
  }

  res_status(&status);
  printf("[Stress 2] Final check after all stress workers finished:\n");
  for (int r = 0; r < status.num_resources; r++) {
    printf("  Resource %d: Total=%d, Available=%d\n",
           status.resources[r].id, status.resources[r].total, status.resources[r].available);
  }

  printf("=== Resource Stress Test Passed! ===\n\n");
  exit(0);
}
