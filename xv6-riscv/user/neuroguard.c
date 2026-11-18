#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/resman.h"

void
print_header(void)
{
  printf("\n=======================================================\n");
  printf("   NEUROGUARD: AI-Powered Deadlock Prevention in xv6   \n");
  printf("=======================================================\n");
}

int
main(int argc, char *argv[])
{
  print_header();

  printf("Running Complete NeuroGuard Demonstration Suite...\n\n");

  printf("--- Step 1: Safe Resource Allocation ---\n");
  int pid = fork();
  if (pid == 0) {
    char *args[] = { "safe_test", 0 };
    exec("safe_test", args);
    exit(0);
  }
  wait(0);

  pause(10);

  printf("\n--- Step 2: High Contention & Dynamic ML Risk Scoring ---\n");
  pid = fork();
  if (pid == 0) {
    char *args[] = { "contention", 0 };
    exec("contention", args);
    exit(0);
  }
  wait(0);

  pause(10);

  printf("\n--- Step 3: Resource Stress & Auto-Reclaim on Exit ---\n");
  pid = fork();
  if (pid == 0) {
    char *args[] = { "stress_test", 0 };
    exec("stress_test", args);
    exit(0);
  }
  wait(0);

  pause(10);

  printf("\n--- Step 4: AI Deadlock Prevention Intervention ---\n");
  pid = fork();
  if (pid == 0) {
    char *args[] = { "deadlock_test", "1", 0 };
    exec("deadlock_test", args);
    exit(0);
  }
  wait(0);

  printf("\n=======================================================\n");
  printf("   ALL NEUROGUARD KERNEL DEMONSTRATIONS COMPLETED!     \n");
  printf("=======================================================\n\n");

  exit(0);
}
