#include <libinput.h>
#include <libudev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int
input_open_restricted(const char *path, int flags, void *user_data)
{
	return open(path, flags);
}
static void
input_close_restricted(int fd, void *user_data)
{
	close(fd);
}
static struct libinput_interface input_implementation = {
	.open_restricted = input_open_restricted,
	.close_restricted = input_close_restricted
};

int main()
{
	struct udev *udev_context = udev_new();
	libinput_udev_create_context(input_implementation, udev_context);
}
