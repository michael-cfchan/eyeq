#include <linux/version.h>
#include <linux/module.h>
#include "vq.h"
#include "rx.h"
#include "tx.h"
#include "stats.h"

MODULE_AUTHOR("Vimal <j.vimal@gmail.com>");
MODULE_DESCRIPTION("Perf Isolation");
MODULE_VERSION("1");
MODULE_LICENSE("GPL");

char *iso_param_dev;
MODULE_PARM_DESC(iso_param_dev, "Interface to operate perfiso.");
module_param(iso_param_dev, charp, 0);

/* This is the main device (should be in the root ns) to be used for
 * packet tx.  We need it irrespective of how packets are classified.
 * It will be used to directly transmit feedback packets, without any
 * routing.  Eventually, we may want a kernel UDP socket, but that's
 * for later.
 */
struct net_device *iso_netdev;
static int iso_init(void);
static void iso_exit(void);

static int iso_init() {
	int i, ret = -1;

	if(iso_param_dev == NULL) {
		/*
		  printk(KERN_INFO "perfiso: need iso_param_dev, the interface to protect.\n");
		  goto err;
		*/
		iso_param_dev = "peth2\0";
	}

	/* trim */
	for(i = 0; i < 32 && iso_param_dev[i] != '\0'; i++) {
		if(iso_param_dev[i] == '\n') {
			iso_param_dev[i] = '\0';
			break;
		}
	}

	rcu_read_lock();
	iso_netdev = dev_get_by_name_rcu(&init_net, iso_param_dev);
	if(iso_netdev == NULL) {
		printk(KERN_INFO "perfiso: device %s not found", iso_param_dev);
		goto out;
	}

	printk(KERN_INFO "perfiso: operating on %s (%p)\n",
		   iso_param_dev, iso_netdev);

	if(iso_params_init())
		goto out;

	if(iso_rx_init())
		goto out;

	if(iso_tx_init())
		goto out;

	if(iso_stats_init())
		goto out;

	ret = 0;
 out:
	rcu_read_unlock();
	return ret;
}

static void iso_exit() {
	iso_stats_exit();
	iso_tx_exit();
	iso_rx_exit();
	iso_params_exit();
	dev_put(iso_netdev);

	printk(KERN_INFO "perfiso: goodbye.\n");
}

module_init(iso_init);
module_exit(iso_exit);

/* Local Variables: */
/* indent-tabs-mode:t */
/* End: */

