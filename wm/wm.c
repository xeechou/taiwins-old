#include "wm.h"
#include <wayland-util.h>


static int
wm_init_monitors(wm_monitor *mons, int args)
{
	int n_mons;
	//set how many monitors, and their geomitry info
	//return n monitors we need
	return n_mons;
}

/** This function get a level of a client for the given monitor(client_manager)
 * @manager: the client is in this manager
 * @c: we want to know this client's level
 */
static int
level_of_client(wm_clients_man *manager, wm_client *c)
{
	/* XXX: This can be done by examing the location of clients */
	int i;
	float accum_len;	/* the accumlation of width/height of the level */

	for (i = 0, accum_len = 0.0; i < NLEVEL; i++) {
		if (c->x == accum_len)
			break;
		accum_len += manager->s_each[i];
	}
	return i;
}

static wm_client *
_leader_of_level(wm_clients_man *manager, int level)
{
	u_list *iterator = manager->clients;

	int j;
	for (j = 0; j < level; j++) {
		if (0 == manager->n_each[j])
			break;		/* there is no next level */

		/* iterator will be the leader of next level after each inner
		 * loop.
		 */
		int i;
		for (i = 0; i < manager->n_each[j]; i++)
			iterator = iterator->next;
	}
	if (iterator == manager->clients)
		return NULL;
	return client_of(iterator->next);
}

static inline wm_client *
leader_of_level(wm_clients_man *manager, int level)
{
	if (!manager->clients)
		return NULL;
	level = MIN( MAX(level, 0),  NLEVEL);
	return _leader_of_level(manager, level);
}

/* increase/decrease the level for current client */
static int
increase_level(wm_arg *argument)
{
	int mon_id = globals.selmon->id;
	wm_clients_man *manager = &(globals.selmon->recent[0]->managers[mon_id]);
	wm_client *c =  manager->selc;
	int level = level_of_client(manager, c);
	//max and min has to be done here
	wm_client *leader = leader_of_level(manager, MAX(level + argument->i, 0));

	//TODO
	//put the client to the given list
	//arrange
}

static void
wm_init_client_manager(wm_clients_man *manager)
{
	*manager = (wm_clients_man){
		.n_each = {0},
		.s_each = {0.0},
		.clients = NULL,
		.float_clients = NULL,
		.selc = NULL
	};
}
static inline wm_clients_man *
find_manager(wm_workspace *space, wm_monitor *mon)
{
	//if every workspace access all monitors, we should just index to monitor
	return &(space->managers[mon->id]);
}

static int wm_new_client_list(wm_clients_man *manager)
{
	manager->clients = new_client_list();	//we use unified slot allocater for clients list
}


//col
static int
col_arrange_monitor(wm_workspace *space, wm_monitor *mon)
{
	int level;
	//find the monitor we need to arrange
	wm_clients_man *target_man = find_manager(space, mon);
	u_list *c = target_man->clients;
	//no clients on monitor
	if (!c)
		return 0;
	for (level = 0; level < NLEVEL; level++) {
		int i, n, ww, wh, x;
		float h, w;
		n = target_man->n_each[level];
		w = target_man->s_each[level];
		x = 0;
		if (n == 0)
			break;
		h = (1.0 / n);
		ww = (int) mon->width  * w;
		wh = (int) mon->height * h;

		//arrange clients for each level
		for (i = 0; i < n; i++) {
			//TODO: we have a bar on the top, need to configure it
			//later
			int y = i * wh;
			wm_client *client = client_of(c);
			wm_resize(client, x, y, ww, wh);
		}
		x += ww;
	}
}
