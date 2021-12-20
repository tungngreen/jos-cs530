SETGATE(idt[T_DIVIDE], 1, GD_KT, DIVIDE_F, 0);
	SETGATE(idt[T_DEBUG], 1, GD_KT, DEBUG_F, 0);
	SETGATE(idt[T_NMI], 0, GD_KT, NMI_F, 0);
	SETGATE(idt[T_BRKPT], 1, GD_KT, BRKPT_F, 3);
	SETGATE(idt[T_OFLOW], 1, GD_KT, OFLOW_F, 0);
	SETGATE(idt[T_BOUND], 1, GD_KT, OFLOW_F, 0);
	SETGATE(idt[T_ILLOP], 1, GD_KT, ILLOP_F, 0);
	SETGATE(idt[T_DEVICE], 1, GD_KT, DEVICE_F, 0);
	SETGATE(idt[T_DBLFLT], 1, GD_KT, DBLFLT_F, 0);
	SETGATE(idt[T_TSS],1,GD_KT,TSS_F, 0);
	SETGATE(idt[T_SEGNP],1,GD_KT,SEGNP_F, 0);
	SETGATE(idt[T_STACK],1,GD_KT,STACK_F, 0);
	SETGATE(idt[T_GPFLT],1,GD_KT,GPFLT_F, 0);
	SETGATE(idt[T_PGFLT],1,GD_KT,PGFLT_F, 0);
	SETGATE(idt[T_FPERR],1,GD_KT,FPERR_F, 0);
	SETGATE(idt[T_ALIGN],1,GD_KT,ALIGN_F, 0);
	SETGATE(idt[T_MCHK],1,GD_KT,MCHK_F, 0);
	SETGATE(idt[T_SIMDERR],1,GD_KT,SIMDERR_F, 0);
	SETGATE(idt[T_SYSCALL],1,GD_KT,SYSCALL_F, 3);

extern void DIVIDE_F();
extern void DEBUG_F();
extern void NMI_F();
extern void BRKPT_F();
extern void OFLOW_F();
extern void BOUND_F();
extern void ILLOP_F();
extern void DEVICE_F();
extern void DBLFLT_F();
extern void TSS_F();
extern void SEGNP_F();
extern void STACK_F();
extern void GPFLT_F();
extern void PGFLT_F();
extern void FPERR_F();
extern void ALIGN_F();
extern void MCHK_F();
extern void SIMDERR_F();
extern void SYSCALL_F();
extern void DEFAULT_F();

// uint32_t envs_size = ROUNDUP(sizeof(struct Env) * NENV, PGSIZE);
	// envs = (struct Env*) boot_alloc(envs_size);

	uint32_t env_size = ROUNDUP(sizeof(struct Env) * NENV, PGSIZE);
	envs = boot_alloc(env_size);
	env = envs;

	boot_map_region(pml4e, UENVS, env_size, PADDR(envs), PTE_U);
	boot_map_region(pml4e, (uintptr_t) envs, PGSIZE, PADDR(envs), PTE_W);

TRAPHANDLER_NOEC(DIVIDE_F, T_DIVIDE)
TRAPHANDLER_NOEC(DEBUG_F, T_DEBUG)
TRAPHANDLER_NOEC(NMI_F, T_NMI)
TRAPHANDLER_NOEC(BRKPT_F, T_BRKPT)
TRAPHANDLER_NOEC(OFLOW_F, T_OFLOW)
TRAPHANDLER_NOEC(BOUND_F, T_BOUND)
TRAPHANDLER_NOEC(ILLOP_F, T_ILLOP)
TRAPHANDLER_NOEC(DEVICE_F, T_DEVICE)
TRAPHANDLER(DBLFLT_F, T_DBLFLT)
TRAPHANDLER(TSS_F, T_TSS)
TRAPHANDLER(SEGNP_F, T_SEGNP)
TRAPHANDLER(STACK_F, T_STACK)
TRAPHANDLER(GPFLT_F, T_GPFLT)
TRAPHANDLER(PGFLT_F, T_PGFLT)
TRAPHANDLER_NOEC(FPERR_F, T_FPERR)
TRAPHANDLER(ALIGN_F, T_ALIGN)
TRAPHANDLER_NOEC(MCHK_F, T_MCHK)
TRAPHANDLER_NOEC(SIMDERR_F, T_SIMDERR)
TRAPHANDLER_NOEC(SYSCALL_F, T_SYSCALL)
TRAPHANDLER(DEFAULT_F, T_DEFAULT)

_alltraps:
	subq $8,%rsp
	movq $0, (%rsp)
	mov %ds,(%rsp)		
	subq $8,%rsp
	movq $0, (%rsp)
	mov %es,(%rsp)
	PUSHA
	mov %rsp,%rdi
	call trap
	POPA_
	mov (%rsp),%es
	addq $8, %rsp
	mov (%rsp), %ds
	addq $8, %rsp

if (tf->tf_trapno == T_PGFLT) {
		page_fault_handler(tf);
		return;
	}
	if (tf->tf_trapno==T_BRKPT) {
		monitor(tf);
		return;
	}

	if (tf->tf_trapno == T_SYSCALL) {
		uint64_t syscall_no,a1,a2,a3,a4,a5,ret;
		syscall_no = tf->tf_regs.reg_rax;
		a1 = tf->tf_regs.reg_rdx;
		a2 = tf->tf_regs.reg_rcx;
		a3 = tf->tf_regs.reg_rbx;
		a4 = tf->tf_regs.reg_rdi;
		a5 = tf->tf_regs.reg_rsi;
		ret = syscall(syscall_no,a1,a2,a3,a4,a5);
		tf->tf_regs.reg_rax = ret;
		return;
	}

if ((tf->tf_cs & 3) == 0)
		panic("Page fault occured in kernel mode");