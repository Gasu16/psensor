/*
 * Copyright (C) 2010-2016 jeanfi@gmail.com
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
#ifndef _PSENSOR_UI_UNITY_H_
#define _PSENSOR_UI_UNITY_H_

#include <bool.h>
#include <psensor.h>

#if defined(HAVE_UNITY) && HAVE_UNITY

static inline bool ui_unity_is_supported(void) { return true; }

void ui_unity_launcher_entry_update(struct psensor **);

void ui_unity_init(void);

#else

static inline bool ui_unity_is_supported(void) { return false; }

static inline void
ui_unity_launcher_entry_update(struct psensor **s) {}

static inline void ui_unity_init(void) {}

#endif

#endif
