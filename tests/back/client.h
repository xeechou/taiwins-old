#ifndef BACKGROUND_CLIENT_PROTOCOL_H
#define BACKGROUND_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct background_manger;
struct wl_output;
struct wl_surface;

extern const struct wl_interface background_manger_interface;

#define BACKGROUND_MANGER_SET_BACKGROUND	0

#define BACKGROUND_MANGER_SET_BACKGROUND_SINCE_VERSION	1

static inline void
background_manger_set_user_data(struct background_manger *background_manger, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) background_manger, user_data);
}

static inline void *
background_manger_get_user_data(struct background_manger *background_manger)
{
	return wl_proxy_get_user_data((struct wl_proxy *) background_manger);
}

static inline uint32_t
background_manger_get_version(struct background_manger *background_manger)
{
	return wl_proxy_get_version((struct wl_proxy *) background_manger);
}

static inline void
background_manger_destroy(struct background_manger *background_manger)
{
	wl_proxy_destroy((struct wl_proxy *) background_manger);
}

static inline void
background_manger_set_background(struct background_manger *background_manger, struct wl_output *output, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) background_manger,
			 BACKGROUND_MANGER_SET_BACKGROUND, output, surface);
}

#ifdef  __cplusplus
}
#endif

#endif
