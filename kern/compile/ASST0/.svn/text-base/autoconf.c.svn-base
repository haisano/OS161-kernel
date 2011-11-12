/* Automatically generated; do not edit */
#include <types.h>
#include <lib.h>
#include "autoconf.h"

static void autoconf_beep(struct beep_softc *, int);
static void autoconf_con(struct con_softc *, int);
static void autoconf_emu(struct emu_softc *, int);
static void autoconf_lhd(struct lhd_softc *, int);
static void autoconf_lrandom(struct lrandom_softc *, int);
static void autoconf_lser(struct lser_softc *, int);
static void autoconf_ltimer(struct ltimer_softc *, int);
static void autoconf_ltrace(struct ltrace_softc *, int);
static void autoconf_random(struct random_softc *, int);
static void autoconf_rtclock(struct rtclock_softc *, int);
static int nextunit_beep;
static int nextunit_con;
static int nextunit_emu;
static int nextunit_lhd;
static int nextunit_lrandom;
static int nextunit_lser;
static int nextunit_ltimer;
static int nextunit_ltrace;
static int nextunit_random;
static int nextunit_rtclock;

static
int
tryattach_emu_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct emu_softc *dev;
	int result;

	dev = attach_emu_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("emu%d at lamebus%d", devunit, busunit);
	result = config_emu(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_emu = devunit+1;
	autoconf_emu(dev, devunit);
	return 0;
}

static
int
tryattach_ltrace_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct ltrace_softc *dev;
	int result;

	dev = attach_ltrace_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("ltrace%d at lamebus%d", devunit, busunit);
	result = config_ltrace(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_ltrace = devunit+1;
	autoconf_ltrace(dev, devunit);
	return 0;
}

static
int
tryattach_ltimer_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct ltimer_softc *dev;
	int result;

	dev = attach_ltimer_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("ltimer%d at lamebus%d", devunit, busunit);
	result = config_ltimer(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_ltimer = devunit+1;
	autoconf_ltimer(dev, devunit);
	return 0;
}

static
int
tryattach_lrandom_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct lrandom_softc *dev;
	int result;

	dev = attach_lrandom_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("lrandom%d at lamebus%d", devunit, busunit);
	result = config_lrandom(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_lrandom = devunit+1;
	autoconf_lrandom(dev, devunit);
	return 0;
}

static
int
tryattach_lhd_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct lhd_softc *dev;
	int result;

	dev = attach_lhd_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("lhd%d at lamebus%d", devunit, busunit);
	result = config_lhd(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_lhd = devunit+1;
	autoconf_lhd(dev, devunit);
	return 0;
}

static
int
tryattach_lser_to_lamebus(int devunit, struct lamebus_softc *bus, int busunit)
{
	struct lser_softc *dev;
	int result;

	dev = attach_lser_to_lamebus(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("lser%d at lamebus%d", devunit, busunit);
	result = config_lser(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_lser = devunit+1;
	autoconf_lser(dev, devunit);
	return 0;
}

static
int
tryattach_beep_to_ltimer(int devunit, struct ltimer_softc *bus, int busunit)
{
	struct beep_softc *dev;
	int result;

	dev = attach_beep_to_ltimer(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("beep%d at ltimer%d", devunit, busunit);
	result = config_beep(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_beep = devunit+1;
	autoconf_beep(dev, devunit);
	return 0;
}

static
int
tryattach_con_to_lser(int devunit, struct lser_softc *bus, int busunit)
{
	struct con_softc *dev;
	int result;

	dev = attach_con_to_lser(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("con%d at lser%d", devunit, busunit);
	result = config_con(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_con = devunit+1;
	autoconf_con(dev, devunit);
	return 0;
}

static
int
tryattach_rtclock_to_ltimer(int devunit, struct ltimer_softc *bus, int busunit)
{
	struct rtclock_softc *dev;
	int result;

	dev = attach_rtclock_to_ltimer(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("rtclock%d at ltimer%d", devunit, busunit);
	result = config_rtclock(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_rtclock = devunit+1;
	autoconf_rtclock(dev, devunit);
	return 0;
}

static
int
tryattach_random_to_lrandom(int devunit, struct lrandom_softc *bus, int busunit)
{
	struct random_softc *dev;
	int result;

	dev = attach_random_to_lrandom(devunit, bus);
	if (dev==NULL) {
		return -1;
	}
	kprintf("random%d at lrandom%d", devunit, busunit);
	result = config_random(dev, devunit);
	if (result != 0) {
		kprintf(": %s\n", strerror(result));
		/* should really clean up dev */
		return result;
	}
	kprintf("\n");
	nextunit_random = devunit+1;
	autoconf_random(dev, devunit);
	return 0;
}


static
void
autoconf_con(struct con_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_lser(struct lser_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
	{
		if (nextunit_con <= 0) {
			tryattach_con_to_lser(0, bus, busunit);
		}
	}
}

static
void
autoconf_lhd(struct lhd_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_emu(struct emu_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_random(struct random_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_ltimer(struct ltimer_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
	{
		if (nextunit_beep <= 0) {
			tryattach_beep_to_ltimer(0, bus, busunit);
		}
	}
	{
		if (nextunit_rtclock <= 0) {
			tryattach_rtclock_to_ltimer(0, bus, busunit);
		}
	}
}

void
autoconf_lamebus(struct lamebus_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
	{
		int result, devunit=nextunit_emu;
		do {
			result = tryattach_emu_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
	{
		int result, devunit=nextunit_ltrace;
		do {
			result = tryattach_ltrace_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
	{
		int result, devunit=nextunit_ltimer;
		do {
			result = tryattach_ltimer_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
	{
		int result, devunit=nextunit_lrandom;
		do {
			result = tryattach_lrandom_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
	{
		int result, devunit=nextunit_lhd;
		do {
			result = tryattach_lhd_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
	{
		int result, devunit=nextunit_lser;
		do {
			result = tryattach_lser_to_lamebus(devunit, bus, busunit);
			devunit++;
		} while (result==0);
	}
}

static
void
autoconf_beep(struct beep_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_lrandom(struct lrandom_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
	{
		if (nextunit_random <= 0) {
			tryattach_random_to_lrandom(0, bus, busunit);
		}
	}
}

static
void
autoconf_rtclock(struct rtclock_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

static
void
autoconf_ltrace(struct ltrace_softc *bus, int busunit)
{
	(void)bus; (void)busunit;
}

void
pseudoconfig(void)
{
}

