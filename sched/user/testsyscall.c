#include <inc/lib.h>

void
umain(int argc, char **argv)
{
#ifdef SCHED_PRIORITIES
	int old_priority = sys_get_env_priority(0);
	sys_lower_priority(3);
	int new_priority = sys_get_env_priority(0);
	cprintf("i am environment %08x\n", thisenv->env_id);
	cprintf("OLD PRIORITY: %d\nNEW PRIORITY: %d\n", old_priority, new_priority);
#endif
}
