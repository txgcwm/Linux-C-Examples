#ifndef __CONFFILE_H__
#define __CONFFILE_H__

int write_conf_value(const char *key, char *value, const char *file);

int read_conf_value(const char *key, char *value, const char *file);

#endif
