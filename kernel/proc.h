// Saved registers for kernel context switches.
struct context {//该结构体定义了保存用于内核上下文切换的寄存器的上下文。它包括一些通用寄存器（ra、sp）和保存的调用者保存寄存器（s0 到 s11）
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.前在该CPU上运行的进程
  struct context context;     // swtch() here to enter scheduler().CPU上下文
  int noff;                   // Depth of push_off() nesting.中断嵌套深度
  int intena;                 // Were interrupts enabled before push_off()?中断是否可用的标志
};

extern struct cpu cpus[NCPU];  //这是对外声明的一个cpu结构体数组，表示系统中的所有CPU。8

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {//这个结构体定义了陷阱处理代码（在trampoline.S中）中用于保存陷阱处理的相关信息。
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };//这个枚举类型定义了进程的不同状态

// Per-process state
struct proc {
  struct spinlock lock;        //这个成员是一个自旋锁，用于在访问或修改进程数据时提供同步。

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan  等待的通道
  int killed;                  // If non-zero, have been killed  是否被杀死
  int xstate;                  // Exit status to be returned to parent's wait 这个成员表示进程的退出状态。当父进程等待该进程终止时，该值将作为返回值返回给父进程。
  int pid;                     // Process ID  进程ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process  这个成员指向当前进程的父进程。

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack   它存储进程的内核栈的虚拟地址。
  uint64 sz;                   // Size of process memory (bytes)   这个成员表示进程的内存大小，以字节为单位。
  pagetable_t pagetable;       // User page table   它是指向进程的用户页表的指针，包含虚拟内存和物理内存之间的映射关系。
  struct trapframe *trapframe; // data page for trampoline.S   这个成员指向用于 trampoline.S 中的陷阱处理的 trapframe 数据页。
  struct context context;      // swtch() here to run process  这个成员表示进程的上下文，包括在上下文切换时保存的寄存器。
  struct file *ofile[NOFILE];  // Open files  这个数组存储了进程打开文件的指针。
  struct inode *cwd;           // Current directory  它指向进程的当前工作目录。
  char name[16];               // Process name (debugging)   这个成员存储进程的名称，用于调试或标识目的。

  //for trace  这个成员用于跟踪目的，表示进程的跟踪掩码。
  int trace_mask;   
};
