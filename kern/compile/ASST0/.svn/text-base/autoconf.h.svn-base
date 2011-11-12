/* Automatically generated; do not edit */
#ifndef _AUTOCONF_H_
#define _AUTOCONF_H_

struct lamebus_softc;
struct emu_softc;
struct ltrace_softc;
struct ltimer_softc;
struct lrandom_softc;
struct lhd_softc;
struct lser_softc;
struct beep_softc;
struct con_softc;
struct rtclock_softc;
struct random_softc;

void autoconf_lamebus(struct lamebus_softc *dev, int unit);

struct emu_softc *attach_emu_to_lamebus(int devunit, struct lamebus_softc *bus);
struct ltrace_softc *attach_ltrace_to_lamebus(int devunit, struct lamebus_softc *bus);
struct ltimer_softc *attach_ltimer_to_lamebus(int devunit, struct lamebus_softc *bus);
struct lrandom_softc *attach_lrandom_to_lamebus(int devunit, struct lamebus_softc *bus);
struct lhd_softc *attach_lhd_to_lamebus(int devunit, struct lamebus_softc *bus);
struct lser_softc *attach_lser_to_lamebus(int devunit, struct lamebus_softc *bus);
struct beep_softc *attach_beep_to_ltimer(int devunit, struct ltimer_softc *bus);
struct con_softc *attach_con_to_lser(int devunit, struct lser_softc *bus);
struct rtclock_softc *attach_rtclock_to_ltimer(int devunit, struct ltimer_softc *bus);
struct random_softc *attach_random_to_lrandom(int devunit, struct lrandom_softc *bus);

int config_emu(struct emu_softc *dev, int unit);
int config_ltrace(struct ltrace_softc *dev, int unit);
int config_ltimer(struct ltimer_softc *dev, int unit);
int config_lrandom(struct lrandom_softc *dev, int unit);
int config_lhd(struct lhd_softc *dev, int unit);
int config_lser(struct lser_softc *dev, int unit);
int config_beep(struct beep_softc *dev, int unit);
int config_con(struct con_softc *dev, int unit);
int config_rtclock(struct rtclock_softc *dev, int unit);
int config_random(struct random_softc *dev, int unit);

void pseudoconfig(void);

#endif /* _AUTOCONF_H_ */
