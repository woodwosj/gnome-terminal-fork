/*
 * terminal-window-layout.hh
 *
 * Copyright © 2026 GNOME Terminal Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "terminal-window.hh"

G_BEGIN_DECLS

/**
 * TerminalWindowLayout:
 *
 * Manager for terminal window layout operations.
 * Provides cascading and grid tiling functionality.
 */

#define TERMINAL_TYPE_WINDOW_LAYOUT (terminal_window_layout_get_type ())
#define TERMINAL_WINDOW_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TERMINAL_TYPE_WINDOW_LAYOUT, TerminalWindowLayout))
#define TERMINAL_IS_WINDOW_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TERMINAL_TYPE_WINDOW_LAYOUT))

typedef struct _TerminalWindowLayout TerminalWindowLayout;
typedef struct {
  GObjectClass parent_class;
} TerminalWindowLayoutClass;

GType terminal_window_layout_get_type (void);

typedef enum {
  TERMINAL_WINDOW_LAYOUT_CASCADE,
  TERMINAL_WINDOW_LAYOUT_TILE_HORIZONTAL,
  TERMINAL_WINDOW_LAYOUT_TILE_VERTICAL,
} TerminalWindowLayoutMode;

typedef struct {
  gint x;
  gint y;
  gint width;
  gint height;
} TerminalWindowGeometry;

/**
 * TerminalWindowLayout methods:
 */

TerminalWindowLayout *terminal_window_layout_new (void);

void terminal_window_layout_cascade_windows (TerminalWindowLayout *self,
                                             GList *windows,
                                             GdkMonitor *monitor);

void terminal_window_layout_tile_windows (TerminalWindowLayout *self,
                                         GList *windows,
                                         TerminalWindowLayoutMode mode,
                                         GdkMonitor *monitor);

void terminal_window_layout_apply_geometry (TerminalWindowLayout *self,
                                           TerminalWindow *window,
                                           const TerminalWindowGeometry *geom);

TerminalWindowGeometry terminal_window_layout_calculate_cascade_position (TerminalWindowLayout *self,
                                                                         guint window_index,
                                                                         guint total_windows,
                                                                         GdkRectangle *available_area,
                                                                         gint offset_x,
                                                                         gint offset_y);

G_END_DECLS
