/*
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 *
 * This work is part of the White Rabbit project, a research effort led
 * by CERN, the European Institute for Nuclear Research.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fmc.h>

static int fmc_check_version(unsigned long version, const char *name)
{
	if (__FMC_MAJOR(version) != FMC_MAJOR) {
		pr_err("%s: \"%s\" has wrong major (has %li, expected %i)\n",
		       __func__, name, __FMC_MAJOR(version), FMC_MAJOR);
		return -EINVAL;
	}

	if (__FMC_MINOR(version) != FMC_MINOR)
		pr_info("%s: \"%s\" has wrong minor (has %li, expected %i)\n",
		       __func__, name, __FMC_MINOR(version), FMC_MINOR);
	return 0;
}

static int fmc_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	//struct fmc_device *fdev = to_fmc_device(dev);

	/* FIXME: The MODALIAS */
	add_uevent_var(env, "MODALIAS=%s", "fmc");
	return 0;
}

static int fmc_probe(struct device *dev)
{
	struct fmc_driver *fdrv = to_fmc_driver(dev->driver);
	struct fmc_device *fdev = to_fmc_device(dev);

	return fdrv->probe(fdev);
}

static int fmc_remove(struct device *dev)
{
	struct fmc_driver *fdrv = to_fmc_driver(dev->driver);
	struct fmc_device *fdev = to_fmc_device(dev);

	return fdrv->remove(fdev);
}

static void fmc_shutdown(struct device *dev)
{
	/* not implemented but mandatory */
}

static struct bus_type fmc_bus_type = {
	.name = "fmc",
	.match = fmc_match,
	.uevent = fmc_uevent,
	.probe = fmc_probe,
	.remove = fmc_remove,
	.shutdown = fmc_shutdown,
};

/* Every device must have a release method: provide a default */
static void __fmc_release(struct device *dev){ }

/* This is needed as parent for our devices and dir in sysfs */
struct device fmc_bus = {
	.release = __fmc_release,
	.init_name = "fmc",
};

/*
 * Functions for client modules follow
 */

int fmc_driver_register(struct fmc_driver *drv)
{
	if (fmc_check_version(drv->version, drv->driver.name))
		return -EINVAL;
	drv->driver.bus = &fmc_bus_type;
	return driver_register(&drv->driver);
}
EXPORT_SYMBOL(fmc_driver_register);

void fmc_driver_unregister(struct fmc_driver *drv)
{
	driver_unregister(&drv->driver);
}
EXPORT_SYMBOL(fmc_driver_unregister);

/* When a device is registered, we must read the eeprom and parse FRU */
int fmc_device_register(struct fmc_device *fmc)
{
	static int fmc_index; /* a "unique" name for lame devices */
	uint32_t device_id;
	int ret;

	if (fmc_check_version(fmc->version, fmc->carrier_name))
		return -EINVAL;

	/* make sure it is not initialized, or it complains */
	memset(&fmc->dev.kobj, 0, sizeof(struct kobject));

	device_initialize(&fmc->dev);
	if (!fmc->dev.release)
		fmc->dev.release = __fmc_release;
	if (!fmc->dev.parent)
		fmc->dev.parent = &fmc_bus;

	/* Fill the identification stuff (may fail) */
	fmc_fill_id_info(fmc);

	fmc->dev.bus = &fmc_bus_type;

	/* The name is from mezzanine info or carrier info. Or 0,1,2.. */
	device_id = fmc->device_id;
	if (!device_id) {
		dev_warn(fmc->hwdev, "No device_id filled, using index\n");
		device_id = fmc_index++;
	}
	if (!fmc->mezzanine_name) {
		dev_warn(fmc->hwdev, "No mezzanine_name found\n");
		dev_set_name(&fmc->dev, "fmc-%04x", device_id);
	} else {
		dev_set_name(&fmc->dev, "%s-%04x", fmc->mezzanine_name,
			     device_id);
	}
	ret = device_add(&fmc->dev);
	if (ret < 0) {
		dev_err(fmc->hwdev, "Failed in registering \"%s\"\n",
			fmc->dev.kobj.name);
		fmc_free_id_info(fmc);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL(fmc_device_register);

void fmc_device_unregister(struct fmc_device *fmc)
{
	device_del(&fmc->dev);
	fmc_free_id_info(fmc);
	put_device(&fmc->dev);
}
EXPORT_SYMBOL(fmc_device_unregister);


/* Init and exit are trivial */
static int fmc_init(void)
{
	int err;

	err = device_register(&fmc_bus);
	if (err)
		return err;
	err = bus_register(&fmc_bus_type);
	if (err)
		device_unregister(&fmc_bus);
	return err;
}

static void fmc_exit(void)
{
	bus_unregister(&fmc_bus_type);
	device_unregister(&fmc_bus);
}

module_init(fmc_init);
module_exit(fmc_exit);

MODULE_LICENSE("GPL");
