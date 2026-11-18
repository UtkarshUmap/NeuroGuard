#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "resman.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getmemsize(void)
{
  return myproc()->sz;
}

uint64
sys_res_register(void)
{
  int id, instances;
  argint(0, &id);
  argint(1, &instances);
  return resman_register(id, instances);
}

uint64
sys_res_request(void)
{
  int id, count;
  argint(0, &id);
  argint(1, &count);
  return resman_request(myproc()->pid, id, count);
}

uint64
sys_res_release(void)
{
  int id, count;
  argint(0, &id);
  argint(1, &count);
  return resman_release(myproc()->pid, id, count);
}

static struct res_status_user kstatus;
static struct deadlock_status_user kds;

uint64
sys_res_status(void)
{
  uint64 uaddr;
  argaddr(0, &uaddr);
  if (resman_get_status(&kstatus) < 0)
    return -1;
  struct proc *p = myproc();
  if (copyout(p->pagetable, p->sz, uaddr, (char *)&kstatus, sizeof(kstatus)) < 0)
    return -1;
  return 0;
}

uint64
sys_res_deadlock_status(void)
{
  uint64 uaddr;
  argaddr(0, &uaddr);
  if (resman_get_deadlock_status(&kds) < 0)
    return -1;
  struct proc *p = myproc();
  if (copyout(p->pagetable, p->sz, uaddr, (char *)&kds, sizeof(kds)) < 0)
    return -1;
  return 0;
}

uint64
sys_res_risk_score(void)
{
  return resman_get_risk_score();
}

uint64
sys_res_set_policy(void)
{
  int policy;
  argint(0, &policy);
  return resman_set_policy(policy);
}

