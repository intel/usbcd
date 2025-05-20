#ifndef OPM_H
#define OPM_H

int handle_billboard_enumeration();
int handle_charger_notification(struct udev_device *dev);
int handle_bandwidth_notification(const char *tunnel_event);
void handle_notification(const char *summary, const char *body);

#endif // OPM_H
