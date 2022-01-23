#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "syscall.h" // include header of syscall
#include "traps.h"   // include header of traps

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

/*********************************************************
// Initializes both countSyscall and countTraps arrays
// Assigns each element of the arrays to be 0
void assignArr(struct proc *p) {
  for(int i = 0; i < 22; i++) {
    p->countSyscall[i] = 0;
  }
  for(int j = 0; j < 20; j++) {
    p->countTraps[j] = 0;
  }
}


// array of names of syscall commands
static char *syscallNameCode[] = {
  [SYS_fork]    "SYS_fork",
  [SYS_exit]    "SYS_exit",
  [SYS_wait]    "SYS_wait",
  [SYS_pipe]    "SYS_pipe",
  [SYS_read]    "SYS_read",
  [SYS_kill]    "SYS_kill",
  [SYS_exec]    "SYS_exec",
  [SYS_fstat]   "SYS_fstat",
  [SYS_chdir]   "SYS_chdir ",
  [SYS_dup]     "SYS_dup",
  [SYS_getpid]  "SYS_getpid",
  [SYS_sbrk]    "SYS_sbrk",
  [SYS_sleep]   "SYS_sleep",
  [SYS_uptime]  "SYS_uptime",
  [SYS_open]    "SYS_open",
  [SYS_write]   "SYS_write",
  [SYS_mknod]   "SYS_mknod",
  [SYS_unlink]  "SYS_unlink",
  [SYS_link]    "SYS_link",
  [SYS_mkdir]   "SYS_mkdir",
  [SYS_close]   "SYS_close",
  [SYS_countTraps]    "SYS_countTraps"
  };


// array of names of traps
static char *trapNameCode[] = {
  [T_DIVIDE]   "T_DIVIDE",
  [T_DEBUG]    "T_DEBUG",
  [T_NMI]      "T_NMI",
  [T_BRKPT]    "T_BRKPT",
  [T_OFLOW]    "T_OFLOW",
  [T_BOUND]    "T_BOUND",
  [T_ILLOP]    "T_ILLOP",
  [T_DEVICE]   "T_DEVICE",
  [T_DBLFLT]   "T_DBLFLT",
  [T_TSS-1]    "T_TSS",
  [T_SEGNP-1]  "T_SEGNP",
  [T_STACK-1]  "T_STACK",
  [T_GPFLT-1]  "T_GPFLT",
  [T_PGFLT-1]  "T_PGFLT",
  [T_FPERR-2]  "T_FPERR",
  [T_ALIGN-2]  "T_ALIGN",
  [T_MCHK-2]     "T_MCHK",
  [T_SIMDERR-2]  "T_SIMDERR",
  [18]           "T_SYSCALL",
  [19]           "T_DEFAULT"
  };
*******************************************************/

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;

  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");

  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);

  // assigns array of p
  assignArr(p);

  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  /************************************************
  // adds up both syscalls and trap calls
  // from parent and child process
  int i;
  for(i = 0; i < 22; i++) {
    curproc->parent->countSyscall[i] = curproc->parent->countSyscall[i] + curproc->countSyscall[i];
    if(i < 20) {
      curproc->parent->countTraps[i] = curproc->parent->countTraps[i] + curproc->countTraps[i];
    }
  }
  ************************************************/
  
  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}


// Added: PA2
/*************************************************************************************

// This function is our system call. It keeps track of both system calls and traps calls.
// At first, it counts their occurrence from the current process.
// -> This is saved in countTraps[] and countSyscall[] arrays.
// Then, it displays them to the shell screen.
// After that, it shows the names of each call in both countTraps[] and countSyscall[] arrays,
// and the number of their occurrences in a process next to them.
// At last, it shows how many times the user process has been trapped to the OS in general.

int
countTraps(void) {
  int totalSyscall = 0;
  int totalTraps = 0;

  for(int i = 0; i < 20; i++) {
    totalTraps += myproc()->countTraps[i];
  }

  cprintf("\n*-------------------------------*\n");
  cprintf("| Total number of traps = %d\t|\n", totalTraps);
  cprintf("|-------------------------------|\n");
  cprintf("| Trap Name \t| Occurrences\t|\n");
  cprintf("|-------------------------------|\n");

  for(int j = 0; j < 20; j++) {
    if(myproc()->countTraps[j] != 0) {
      cprintf("| [%s]\t| %d\t\t|\n", trapNameCode[j], myproc()->countTraps[j]);
    }
  }
  cprintf("*-------------------------------*\n");

  for(int m = 0; m < 22; m++) {
    totalSyscall += myproc()->countSyscall[m];
  }

  cprintf("\n*---------------------------------------*\n");
  cprintf("| Total number of system calls = %d\t|\n", totalSyscall);
  cprintf("|---------------------------------------|\n");
  cprintf("| Syscall Name \t\t| Occurrences\t|\n");
  cprintf("|---------------------------------------|\n");

  for(int n = 0; n < 22; n++) {
    if(myproc()->countSyscall[n] != 0) {
      if(n == 21) { // if the call is [SYS_countTraps] ; does a bit of formatting
        cprintf("| [%s] \t| %d\t\t|\n", syscallNameCode[n + 1], myproc()->countSyscall[n]);
      }
      else {
        cprintf("| [%s] \t\t| %d\t\t|\n", syscallNameCode[n + 1], myproc()->countSyscall[n]);
      }
    }
  }
  cprintf("*---------------------------------------*\n");

  cprintf("\n*---------------------------------------*\n");
  if(totalTraps +totalSyscall < 10) {
    cprintf("| Total number of all calls = %d\t\t|\n", totalTraps + totalSyscall);
  }
  else {
    cprintf("| Total number of all calls = %d\t|\n", totalTraps + totalSyscall);
  }
  cprintf("*---------------------------------------*\n\n");

  return 22;
}

*************************************************************************************/
