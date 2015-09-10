/** @file shm.c
 *  @brief implementation for wayland server side of shared memory manager.
 *
 *  shm is a global, can be used to create shared memory pool.
 */
#include <stdbool.h>
#include <stdint.h>
#include <wayland-server.h>
#include "server.h"

struct serv_shm
{
	struct wl_global *global;
	struct wl_shm *shm_obj;
} shm;

struct serv_shm_pool
{
	struct wl_resource *resource;
	void *data;
	uint32_t size;
};

/** @brief this function will be called when wl_registry_bind called.
 */
static void
create_pool(struct wl_client *client, struct wl_resource *resource,
	    uint32_t id, int32_t fd, int32_t size)
{
	/* there are a few things need to be done with this.
	 * 1) malloc a large memory of pool, with size.
	 * 2) mmap shm_pool->data to fd, basically every client has there own
	 * pool.
	 * 3) create wl_pool_interface
	 *
	 *
	 * server need to read that buffer and use it for compositor.
	 */
	return;
}

static const struct wl_shm_interface shm_implementation = {
	create_pool
};

static void
bind_shm(struct wl_client *client, void *data,
	      uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	resource = wl_resource_create(client, &wl_shm_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &shm_implementation,
				       data, NULL);
}

/** @brief setting up wayland shm interface.
 *
 *  luckly, wayland already did that for us.
 */
bool
serv_shm_init(void)
{
	/* this is the wayland implementation,
	 * a version of my own need to be write later
	 */
	wl_display_init_shm(server_tw.display);
	return true;
}

void
serv_shm_destroy(void)
{
	wl_global_destroy(shm.global);
	return;
}
