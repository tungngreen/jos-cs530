// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/dwarf.h>
#include <kern/kdebug.h>
#include <kern/dwarf_api.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display information for debugging", mon_backtrace}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	//cprintf("Stack backtrace:\n");
	//uint64_t *rbp = (uint64_t *)read_rbp();
	//uint64_t rip;
	//read_rip(rip);
	// while (rbp != 0){
    //     cprintf("  rbp %016x  rip %016x\n", rbp, rip);
	// 	struct Ripdebuginfo ripinfo;
	// 	if (debuginfo_rip(rip, &ripinfo) == 0) {
	// 		cprintf("      %s:%d: %.*s+%016x   args:%d", ripinfo.rip_file, ripinfo.rip_line, ripinfo.offset_fn_arg[0], ripinfo.rip_fn_name, ripinfo.rip_fn_narg);
	// 	}
	// 	uint64_t arg = *((uint64_t*)rbp);
	// 	cprintf(" %016x ", arg);
	// 	cprintf("\n");
    //     rbp = (uint64_t *)(*rbp);
    // }
	cprintf("Stack backtrace:\n");
        uint64_t rbp = read_rbp();
        uint64_t rip;
        read_rip(rip);
        while(1) {
                cprintf("rbp %016x rip %016x \n",rbp,rip);
                struct Ripdebuginfo dbg;
                debuginfo_rip(rip,&dbg);
                cprintf("    %s:%d: %s+%016x args:%d ",dbg.rip_file,dbg.rip_line,dbg.rip_fn_name,rip-dbg.rip_fn_addr,dbg.rip_fn_narg);
                uint64_t saved_rbp = rbp;
                for(int i = 0 ;i < dbg.rip_fn_narg;i++) {
                        switch(dbg.size_fn_arg[i]) {
                                case 4:
                                        {
                                                int args = *(int*)(saved_rbp - 4);
                                                cprintf(" %016x ",args);
                                                saved_rbp -= 4;
                                                break;
                                        }
                                case 8:
                                        {
                                                uint64_t args = *(uint64_t*)(saved_rbp - 4);
                                                cprintf(" %016x ",args);
                                                saved_rbp -= 8;
                                                break;
                                        }
                        }
                }
                cprintf("\n");
                rip = *(uint64_t*)(rbp + 8);
                rbp = *(uint64_t*)rbp;
                if(rbp == 0)
                        break;
		}
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
