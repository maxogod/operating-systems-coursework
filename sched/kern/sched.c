#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define SCHED_CYCLES_UNTIL_BOOST 5  // Cycles until all envs are boosted
#define SCHED_ALLOTMENT 2           // Cycles allotment for each priority level

// Stats
unsigned int sched_times_run = 0;
unsigned int sched_times_boosted = 0;

void sched_mlfq(void);
void sched_round_robin(envpriority_t priority);
void sched_halt(void);
void sched_boost_priorities(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	sched_times_run++;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// All environments will have the same priority
	sched_round_robin(ENV_MAX_PRIORITY);
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	sched_mlfq();
#endif

	sched_halt();
}

void
sched_mlfq(void)
{
	// Rule 5: After some time period S, move all the jobs in the system
	// to the topmost queue.
	sched_boost_priorities();

	// Rule 4: Once a job uses up its time allotment at a given level
	// (regardless of how many times it has given up the CPU),
	// its priority is reduced.
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs >= SCHED_ALLOTMENT &&
		    envs[i].env_priority < ENV_MIN_PRIORITY) {
			envs[i].env_priority++;
			envs[i].env_times_downgraded++;
			envs[i].env_runs = 0;
		}
	}

	// Rule 1: If Priority(A) > Priority(B), A runs (B doesn’t).
	// Rule 2: If Priority(A) = Priority(B), A & B run in round-robin
	for (envpriority_t priority = ENV_MAX_PRIORITY;
	     priority <= ENV_MIN_PRIORITY;
	     priority++) {
		sched_round_robin(priority);
	}
}

void
sched_round_robin(envpriority_t priority)
{
	int start = curenv ? ENVX(curenv->env_id) + 1 : 0;

	for (int i = start; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE &&
		    envs[i].env_priority == priority) {
			env_run(&envs[i]);
		}
	}

	// If no runnable environment is found after the current position,
	// iterate from the beginning of the array to the position before start
	for (int i = 0; i < start; i++) {
		if (envs[i].env_status == ENV_RUNNABLE &&
		    envs[i].env_priority == priority) {
			env_run(&envs[i]);
		}
	}

	// If no runnable environments were found, check if the previous one is still running
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}
}

void
sched_boost_priorities(void)
{
	if (sched_times_run % SCHED_CYCLES_UNTIL_BOOST == 0) {
		for (int i = 0; i < NENV; i++) {
			envs[i].env_priority = ENV_MAX_PRIORITY;
			envs[i].env_runs = 0;
			envs[i].env_times_boosted++;
		}
		sched_times_boosted++;
		return;
	}
}

void
print_stats()
{
	cprintf("\n=> Scheduler statistics: \n");

	cprintf("* Times schedule was called: %d\n", sched_times_run);
	cprintf("* Times schedule has boosted priorities: %d\n",
	        sched_times_boosted);

	cprintf("\n=> Processes statistics: \n");

	cprintf("Env Id \t | runs \t | boosts \t | downgrades"
	        "\n");
	cprintf("—————————————————————————————————————————————————————————————"
	        "\n");

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_total_runs != 0) {
			cprintf("%d \t | %d \t | %d \t | %d \n",
			        envs[i].env_id,
			        envs[i].env_total_runs,
			        envs[i].env_times_boosted,
			        envs[i].env_times_downgraded);
		};
	}
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		print_stats();
		while (1)
			monitor(NULL);
	}


	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance.


	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
