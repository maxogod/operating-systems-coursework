#include <inc/lib.h>

void
umain(int argc, char **argv)
{
#ifdef SCHED_PRIORITIES
	cprintf("Setting priority of %08x to 3\n", thisenv->env_id);
	sys_lower_priority(3);
	int old_priority = sys_get_env_priority(0);
	cprintf("Setting priority of %08x to 0\n", thisenv->env_id);
	sys_lower_priority(0);
	int new_priority = sys_get_env_priority(0);
	cprintf("OLD PRIORITY: %d\nNEW PRIORITY: %d\n", old_priority, new_priority);
#endif
}
