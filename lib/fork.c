// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);
extern volatile pte_t uvpt[];
extern volatile pde_t uvpd[];
extern volatile pde_t uvpde[];  
extern volatile pde_t uvpml4e[];

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	// pte_t pte = uvpt[VPN((uintptr_t) addr)];
	// pte_t ptd = uvpd[VPD(addr)];
	// pte_t ptde = uvpd[VPDPE(addr)];
	// pte_t pml4e = uvpd[VPML4E(addr)];
    // envid_t envid = sys_getenvid();


	//pgfault() checks for FEC_WR in the error code to make sure that this is a write
	// cprintf("env id %08x\n",envid);
	// if (!(FEC_WR & err)) {
	// 	panic("pgfault: not a write\n");
	// }
	// if (!(PTE_COW & pte)) {
	// 	panic("pgfault: not a copy-on-write\n");

	// }

	// //Allocate a new page, map it at a temporary location (PFTEMP)
	// r = sys_page_alloc(0, (void *)PFTEMP, (PTE_W | PTE_U | PTE_P));
	// if (r < 0) {
	// 	panic("pgfault: %e", r);
	// }

	// // copy the data from the old page to the new page
	// memmove((void *) PFTEMP, (void *) ROUNDDOWN(addr, PGSIZE), PGSIZE);
	// r = sys_page_map(0, (void *)PFTEMP, 0, (void *)ROUNDDOWN(addr, PGSIZE), (PTE_W | PTE_U | PTE_P));
	// if (r < 0) {
	// 	panic("pgfault: %e", r);
	// }

	// //unmap the temporary page
	// r = sys_page_unmap(0, (void *)PFTEMP);
	// if (r < 0) {
	// 	panic("pgfault: %e", r);
	// }

	pte_t pte = uvpt[PGNUM(addr)];
    envid_t envid = thisenv->env_id;

	//pgfault() checks for FEC_WR in the error code to make sure that this is a write
	if ((FEC_WR & err) == 0) {
		panic("pgfault: not a write\n");
	}
	if ((PTE_COW & uvpt[PGNUM((uintptr_t)addr)]) == 0) {
		panic("pgfault: not a copy-on-write\n");

	}

	//Allocate a new page, map it at a temporary location (PFTEMP)
	r = sys_page_alloc(0, PFTEMP, (PTE_W | PTE_U | PTE_P));
	if (r < 0) {
		panic("pgfault: %e", r);
	}

	// copy the data from the old page to the new page
	memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
	r = sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), (PTE_W | PTE_U | PTE_P));
	if (r < 0) {
		panic("pgfault: %e", r);
	}

	//unmap the temporary page
	r = sys_page_unmap(0, PFTEMP);
	if (r < 0) {
		panic("pgfault: %e", r);
	}

}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//

static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	uintptr_t va = pn * PGSIZE;
	int ret;
	if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)) {
		
		ret = sys_page_map(thisenv->env_id, (void *) va, envid, (void *) va, PTE_COW | PTE_U | PTE_P);
		if (ret < 0) {
			panic("duppage: %e", ret);
		}
		ret = sys_page_map(thisenv->env_id, (void *) va, thisenv->env_id, (void *) va, PTE_COW | PTE_U | PTE_P);
		if (ret < 0) {
			panic("duppage: %e", ret);
		}
	} else {
		ret = sys_page_map(thisenv->env_id, (void *) va, envid, (void *) va, PTE_U | PTE_P);
		if (ret < 0) {
			panic("duppage: %e", ret);
		}
	}
	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.

	//The parent install pgfault using set_pgfault_handler().
	set_pgfault_handler(pgfault);

	envid_t child_envid = sys_exofork();

	if (child_envid < 0) { //failed to create a child
		panic("fork: %e", child_envid);
	}
	if (child_envid == 0) { //this is the child
		// Remember to fix "thisenv" in the child process.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (uintptr_t addr = 0; addr < USTACKTOP; addr += PGSIZE) {
		//if (((uvpd[VPD(addr)] & PTE_P) == PTE_P) && ((uvpt[PGNUM(addr)] & PTE_P) == PTE_P)) {
		if (!(uvpml4e[VPML4E(addr)] & PTE_P)) {
			continue;
		}	
		if (!(uvpde[VPDPE(addr)] & PTE_P)) {
			continue;
		}

		if (!(uvpd[VPD(addr)] & PTE_P)) {
			continue;
		}
		if (uvpt[PGNUM(addr)] & PTE_P) {
			duppage(child_envid, PGNUM(addr));
		}

	}

	int ret = sys_page_alloc(child_envid, (void *) (UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W);
	if (ret < 0) {
		panic("fork: %e", ret);
	}
	ret = sys_env_set_pgfault_upcall(child_envid, _pgfault_upcall);
	if (ret < 0) {
		panic("fork: %e", ret);
	}

	ret = sys_env_set_status(child_envid, ENV_RUNNABLE);
	if (ret < 0) {
		panic("fork: %e", ret);
	}

	
	// Returns: child's envid to the parent
	return child_envid;

	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
