#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

struct tw_drm_context {
	struct udev *udev_context;
	const char *seat_id;
	char *gpu_filename;
};

static const char *default_seat = "seat0";

bool find_primary_gpu(struct tw_drm_context *c)
{
	struct udev_enumerate *e;
	struct udev_list_entry *entry;
	const char *path, *device_seat, *id, *filename;
	struct udev_device *device, *primary_gpu, *pci;


	e = udev_enumerate_new(c->udev_context);
	udev_enumerate_add_match_subsystem(e, "drm");
	udev_enumerate_add_match_sysname(e, "card[0-9]");
	udev_enumerate_scan_devices(e);

	primary_gpu = NULL;
	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)) {
		path = udev_list_entry_get_name(entry);
		device = udev_device_new_from_syspath(c->udev_context, path);
		if (!device)
			continue;
		device_seat = udev_device_get_property_value(device, "ID_SEAT");
		if (!device_seat)
			device_seat = default_seat;
		//systemd made graphic card attached to seat, but in our case,
		//only one seats
		if (strcmp(device_seat, c->seat_id)) {
			udev_device_unref(device);
			continue;
		}

		pci = udev_device_get_parent_with_subsystem_devtype(device,
								"pci", NULL);
		if (pci) {
			id = udev_device_get_sysattr_value(pci, "boot_vga");
			if (id && !strcmp(id, "1")) {
				if (primary_gpu)
					udev_device_unref(primary_gpu);
				primary_gpu = device;
				break;
			}
		}
		if (!primary_gpu)
			primary_gpu = device;


		udev_device_unref(device);
	}

	udev_enumerate_unref(e);
	if (!primary_gpu)
		return false;

	filename = udev_device_get_devnode(primary_gpu);
	c->gpu_filename = strdup(filename);
	return true;
}


int main()
{
	struct tw_drm_context context = {.seat_id = default_seat};
	context.udev_context = udev_new();
	if (!find_primary_gpu(&context))
		fprintf(stderr, "could not find suitable gpu\n");
	else
		fprintf(stderr, "valid gpu is %s\n", context.gpu_filename);

	udev_unref(context.udev_context);
	free(context.gpu_filename);
}
