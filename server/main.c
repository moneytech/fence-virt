#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

/* Local includes */
#include <simpleconfig.h>
#include <server_plugin.h>
#include <debug.h>

extern fence_callbacks_t libvirt_callbacks; /* should be in a header */

int
main(int argc, char **argv)
{
	char val[80];
	config_object_t *config;
	const plugin_t *p;
	srv_context_t mcast_context;
	srv_context_t libvirt_context; /*XXX these should be differently
					 named context types */
	int debug_set = 0;
	int opt;

	config = sc_init();

	while ((opt = getopt(argc, argv, "f:d:")) != EOF) {
		switch(opt) {
		case 'f':
			printf("Using %s\n", optarg);
			if (sc_parse(config, optarg) != 0) {
				printf("Failed to parse %s\n", optarg);
				return -1;
			}
			break;
		case 'd':
			dset(atoi(optarg));
			debug_set = 1;
			break;
		default: 
			break;
		}
	}


	if (!debug_set) {
		if (sc_get(config, "fence_virtd/@debug",
			   val, sizeof(val)) == 0)
			dset(atoi(val));
	}

	sc_dump(config, stdout);

	if (sc_get(config, "fence_virtd/@backend", val, sizeof(val))) {
		printf("Failed to determine backend.\n");
		printf("%s\n", val);
		return -1;
	}

	printf("Backend plugin: %s\n", val);

#ifdef _MODULE
	if (plugin_load("./libvirt.so") < 0) {
		printf("Doom\n");
	}
#endif
	plugin_dump();

	p = plugin_find(val);
	if (!p) {
		printf("Could not find plugin \"%s\n", val);
	}

	if (p->init(&libvirt_context, config) < 0) {
		printf("%s failed to initialize\n", val);
		return 1;
	}

	/* only client we have now is mcast (fence_xvm behavior) */
	if (mcast_init(&mcast_context, p->callbacks, config,
		       libvirt_context) < 0) {
		printf("Failed initialization!\n");
		return 1;
	}

	while (mcast_dispatch(mcast_context, NULL) >= 0);

	mcast_shutdown(mcast_context);
	p->cleanup(libvirt_context);

	return 0;
}