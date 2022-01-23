#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

// Added: PA2
/****************************************************************************
// Checks which trap is called based on the current process's trap number.
// The function then increments that trap number by one each time to give us information at the end about
// what types of traps have occurred and the number of times they (traps) have occurred.

void
check_trap_func(struct trapframe *tf)
{
  if(tf->trapno == T_DIVIDE) {
    myproc()->countTraps[0] = myproc()->countTraps[0] + 1;
  }
  else if(tf->trapno == T_DEBUG) {
    myproc()->countTraps[1] = myproc()->countTraps[1] + 1;
  }
  else if(tf->trapno == T_NMI) {
    myproc()->countTraps[2] = myproc()->countTraps[2] + 1;
  }
  else if(tf->trapno == T_BRKPT) {
    myproc()->countTraps[3] = myproc()->countTraps[3] + 1;
  }
  else if(tf->trapno == T_OFLOW) {
    myproc()->countTraps[4] = myproc()->countTraps[4] + 1;
  }
  else if(tf->trapno == T_BOUND) {
    myproc()->countTraps[5] = myproc()->countTraps[5] + 1;
  }
  else if(tf->trapno == T_ILLOP) {
    myproc()->countTraps[6] = myproc()->countTraps[6] + 1;
  }
  else if(tf->trapno == T_DEVICE) {
    myproc()->countTraps[7] = myproc()->countTraps[7] + 1;
  }
  else if(tf->trapno == T_DBLFLT) {
    myproc()->countTraps[8] = myproc()->countTraps[8] + 1;
  }
  else if(tf->trapno == T_TSS) {
    myproc()->countTraps[9] = myproc()->countTraps[9] + 1;
  }
  else if(tf->trapno == T_SEGNP) {
    myproc()->countTraps[10] = myproc()->countTraps[10] + 1;
  }
  else if(tf->trapno == T_STACK) {
    myproc()->countTraps[11] = myproc()->countTraps[11] + 1;
  }
  else if(tf->trapno == T_GPFLT) {
    myproc()->countTraps[12] = myproc()->countTraps[12] + 1;
  }
  else if(tf->trapno == T_PGFLT) {
    myproc()->countTraps[13] = myproc()->countTraps[13] + 1;
  }
  else if(tf->trapno == T_FPERR) {
    myproc()->countTraps[14] = myproc()->countTraps[14] + 1;
  }
  else if(tf->trapno == T_ALIGN) {
    myproc()->countTraps[15] = myproc()->countTraps[15] + 1;
  }
  else if(tf->trapno == T_MCHK) {
    myproc()->countTraps[16] = myproc()->countTraps[16] + 1;
  }
  else if(tf->trapno == T_SIMDERR) {
    myproc()->countTraps[17] = myproc()->countTraps[17] + 1;
  }
  else if(tf->trapno == T_SYSCALL) {
    myproc()->countTraps[18] = myproc()->countTraps[18] + 1;
  }
  else if(tf->trapno == T_DEFAULT) {
    myproc()->countTraps[19] = myproc()->countTraps[19] + 1;
  }
}
****************************************************************************/

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();

      // Added: PA2
      /********************************************************
	  // Both system call and trap number are handled.
      // If a trap number exists, then check_trap_func()
      // is invoked and the new trap call increments
      // the occurrence of that trap by 1 in array countTraps[20].
      // If the eax is valid, then the system call occurrence
      // is incremented by 1in array countSyscall[22].
	
      int num;
      int i;
      num = tf->eax;
      i = num - 1;
      myproc()->tf = tf;
      myproc()->countSyscall[i] = myproc()->countSyscall[i] + 1;
      check_trap_func(myproc()->tf);
      ********************************************************/
	
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
