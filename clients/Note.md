##Do experiment on mmap

##how to let client connect to it.

Wayland protocol is message based, a message from server to client is event. a
message from client to server is request.

For request, clients do not need to implement anything, for event, server does
not need to implement anything. 


##How new interface get announced

registre a global, by wl_global_create(wl_display, interface, bind_function).
bind_function is the one for creating object for a client, bind function will
bind a implementation to a client that listen to its call.

either I need to announce a global in the server. Or I need to create a resource
in a request code

Now, I need to announce the taiwins_shell.

