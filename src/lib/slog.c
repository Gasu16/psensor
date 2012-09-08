/*
 * Copyright (C) 2010-2012 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <locale.h>
#include <libintl.h>
#define _(str) gettext(str)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "bool.h"
#include "config.h"
#include "log.h"
#include "slog.h"

static FILE *file;
static struct timeval stv;
static double *last_values;
static int period;
static struct psensor **sensors;
static pthread_mutex_t *sensors_mutex;
static pthread_t thread;

static const char *DEFAULT_FILENAME = "sensors.log";

static char *get_default_path()
{
	char *home, *path, *dir;

	home = getenv("HOME");

	if (home) {
		dir = malloc(strlen(home)+1+strlen(".psensor")+1);
		sprintf(dir, "%s/%s", home, ".psensor");
		mkdir(dir, 0777);

		path = malloc(strlen(dir)+1+strlen(DEFAULT_FILENAME)+1);
		sprintf(path, "%s/%s", dir, DEFAULT_FILENAME);

		free(dir);

		return path;
	} else {
		log_warn(_("HOME variable not set."));
		return strdup(DEFAULT_FILENAME);
	}
}

static bool slog_open(const char *path, struct psensor **sensors)
{
	char *lpath;

	if (file) {
		log_err(_("Sensor log file already open."));
		return 0;
	}

	lpath = path ? (char *)path : get_default_path();

	file = fopen(lpath, "a");

	if (!file)
		log_err(_("Cannot open sensor log file: %s."), lpath);

	if (!path)
		free((char *)lpath);

	if (!file)
		return 0;

	if (gettimeofday(&stv, NULL)) {
		log_err(_("gettimeofday failed."));
		return 0;
	}

	fprintf(file, "I,%ld,%s\n", stv.tv_sec, VERSION);

	while (*sensors) {
		fprintf(file, "S,%s,%x\n", (*sensors)->id,  (*sensors)->type);
		sensors++;
	}

	fflush(file);

	return 1;
}

static void slog_write_sensors(struct psensor **sensors)
{
	int count, i;
	double v;
	struct timeval tv;
	bool first_call;

	if (!file) {
		log_err(_("Sensor log file not open."));
		return ;
	}

	gettimeofday(&tv, NULL);

	count = psensor_list_size(sensors);

	if (last_values) {
		first_call = 0;
	} else {
		first_call = 1;
		last_values = malloc(count * sizeof(double));
	}

	fprintf(file, "%ld", tv.tv_sec - stv.tv_sec);
	for (i = 0; i < count; i++) {
		v = psensor_get_current_value(sensors[i]);

		if (!first_call && last_values[i] == v)
			fputc(',', file);
		else
			fprintf(file, ",%.1f", v);

		last_values[i] = v;
	}

	fputc('\n', file);

	fflush(file);
}

static void *slog_routine(void *data)
{
	while (1) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(sensors_mutex);
		slog_write_sensors(sensors);
		pthread_mutex_unlock(sensors_mutex);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		sleep(period);
	}

	pthread_exit(0);
}

void slog_close()
{
	if (file) {
		pthread_cancel(thread);

		fclose(file);
		file = NULL;
		free(last_values);
		last_values = NULL;
	} else {
		log_err(_("Sensor log not open, cannot close."));
	}
}

bool slog_activate(const char *path,
		   struct psensor **ss,
		   pthread_mutex_t *mutex,
		   int p)
{
	bool ret;

	sensors = ss;
	sensors_mutex = mutex;
	period = p;

	pthread_mutex_lock(mutex);
	ret = slog_open(path, sensors);
	pthread_mutex_unlock(mutex);

	if (ret)
		pthread_create(&thread, NULL, slog_routine, NULL);

	return ret;
}