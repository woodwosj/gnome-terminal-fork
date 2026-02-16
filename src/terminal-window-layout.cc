/*
 * terminal-window-layout.cc
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

#include "config.h"

#include "terminal-window-layout.hh"

#include <glib/gi18n.h>
#include <math.h>

struct _TerminalWindowLayout {
  GObject parent_instance;
};

G_DEFINE_TYPE (TerminalWindowLayout, terminal_window_layout, G_TYPE_OBJECT)

/* Configuration */
#define WINDOW_CASCADE_OFFSET_X 30
#define WINDOW_CASCADE_OFFSET_Y 30
#define WINDOW_PADDING 10

/* ---- Initialization ---- */

static void
terminal_window_layout_finalize (GObject *object)
{
  G_OBJECT_CLASS (terminal_window_layout_parent_class)->finalize (object);
}

static void
terminal_window_layout_class_init (TerminalWindowLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = terminal_window_layout_finalize;
}

static void
terminal_window_layout_init (TerminalWindowLayout *self)
{
}

/* ---- Public API ---- */

/**
 * terminal_window_layout_new:
 *
 * Creates a new window layout manager.
 *
 * Returns: (transfer full): a new #TerminalWindowLayout
 */
TerminalWindowLayout *
terminal_window_layout_new (void)
{
  return (TerminalWindowLayout *)g_object_new (TERMINAL_TYPE_WINDOW_LAYOUT, nullptr);
}

/**
 * terminal_window_layout_calculate_cascade_position:
 * @self: a #TerminalWindowLayout
 * @window_index: index of window in cascade (0-based)
 * @total_windows: total number of windows
 * @available_area: available screen area
 * @offset_x: horizontal cascade offset
 * @offset_y: vertical cascade offset
 *
 * Calculates the geometry for a window in cascade layout.
 *
 * Returns: a #TerminalWindowGeometry with position and size
 */
TerminalWindowGeometry
terminal_window_layout_calculate_cascade_position (TerminalWindowLayout *self,
                                                   guint window_index,
                                                   guint total_windows,
                                                   GdkRectangle *available_area,
                                                   gint offset_x,
                                                   gint offset_y)
{
  TerminalWindowGeometry geom = {0};

  g_return_val_if_fail (TERMINAL_IS_WINDOW_LAYOUT (self), geom);
  g_return_val_if_fail (available_area != nullptr, geom);

  /* Calculate base position */
  gint x = available_area->x + (window_index * offset_x);
  gint y = available_area->y + (window_index * offset_y);

  /* Clamp to screen boundaries */
  gint max_x = available_area->x + available_area->width;
  gint max_y = available_area->y + available_area->height;

  /* Default cascade width/height - 70% of available area */
  gint width = (available_area->width * 70) / 100;
  gint height = (available_area->height * 70) / 100;

  /* Adjust if window goes off-screen */
  if (x + width > max_x)
    x = max_x - width - WINDOW_PADDING;
  if (y + height > max_y)
    y = max_y - height - WINDOW_PADDING;

  /* Ensure minimum boundaries */
  if (x < available_area->x)
    x = available_area->x + WINDOW_PADDING;
  if (y < available_area->y)
    y = available_area->y + WINDOW_PADDING;

  geom.x = x;
  geom.y = y;
  geom.width = width;
  geom.height = height;

  return geom;
}

/**
 * terminal_window_layout_apply_geometry:
 * @self: a #TerminalWindowLayout
 * @window: a #TerminalWindow
 * @geom: a #TerminalWindowGeometry
 *
 * Applies the given geometry to a window.
 */
void
terminal_window_layout_apply_geometry (TerminalWindowLayout *self,
                                       TerminalWindow *window,
                                       const TerminalWindowGeometry *geom)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_LAYOUT (self));
  g_return_if_fail (TERMINAL_IS_WINDOW (window));
  g_return_if_fail (geom != nullptr);

  gtk_window_move (GTK_WINDOW (window), geom->x, geom->y);
  gtk_window_set_default_size (GTK_WINDOW (window), geom->width, geom->height);
}

/**
 * terminal_window_layout_cascade_windows:
 * @self: a #TerminalWindowLayout
 * @windows: (element-type TerminalWindow): list of windows
 * @monitor: a #GdkMonitor or %NULL
 *
 * Arranges windows in a cascade pattern.
 */
void
terminal_window_layout_cascade_windows (TerminalWindowLayout *self,
                                        GList *windows,
                                        GdkMonitor *monitor)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_LAYOUT (self));

  if (windows == nullptr)
    return;

  /* Get available monitor area */
  GdkRectangle available_area = {0};
  if (monitor != nullptr)
    {
      gdk_monitor_get_geometry (monitor, &available_area);
    }
  else
    {
      /* Use primary monitor or first available */
      GdkDisplay *display = gdk_display_get_default ();
      monitor = gdk_display_get_monitor (display, 0);
      if (monitor)
        gdk_monitor_get_geometry (monitor, &available_area);
    }

  /* Arrange each window in cascade pattern */
  guint index = 0;
  for (GList *l = windows; l != nullptr; l = l->next, index++)
    {
      TerminalWindow *window = TERMINAL_WINDOW (l->data);
      TerminalWindowGeometry geom = terminal_window_layout_calculate_cascade_position (
        self, index, g_list_length (windows),
        &available_area,
        WINDOW_CASCADE_OFFSET_X,
        WINDOW_CASCADE_OFFSET_Y);

      terminal_window_layout_apply_geometry (self, window, &geom);
    }
}

/**
 * terminal_window_layout_tile_windows:
 * @self: a #TerminalWindowLayout
 * @windows: (element-type TerminalWindow): list of windows
 * @mode: the tiling mode
 * @monitor: a #GdkMonitor or %NULL
 *
 * Arranges windows in a grid pattern (horizontal or vertical).
 */
void
terminal_window_layout_tile_windows (TerminalWindowLayout *self,
                                     GList *windows,
                                     TerminalWindowLayoutMode mode,
                                     GdkMonitor *monitor)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_LAYOUT (self));

  if (windows == nullptr)
    return;

  /* Get available monitor area */
  GdkRectangle available_area = {0};
  if (monitor != nullptr)
    {
      gdk_monitor_get_geometry (monitor, &available_area);
    }
  else
    {
      GdkDisplay *display = gdk_display_get_default ();
      monitor = gdk_display_get_monitor (display, 0);
      if (monitor)
        gdk_monitor_get_geometry (monitor, &available_area);
    }

  guint count = g_list_length (windows);
  if (count == 0)
    return;

  /* Calculate grid dimensions */
  guint rows = 1;
  guint cols = count;

  if (mode == TERMINAL_WINDOW_LAYOUT_TILE_VERTICAL)
    {
      rows = (guint)ceil (sqrt ((double)count));
      cols = (count + rows - 1) / rows;
    }
  else /* HORIZONTAL */
    {
      cols = (guint)ceil (sqrt ((double)count));
      rows = (count + cols - 1) / cols;
    }

  /* Calculate window size */
  gint window_width = (available_area.width - WINDOW_PADDING) / cols - WINDOW_PADDING;
  gint window_height = (available_area.height - WINDOW_PADDING) / rows - WINDOW_PADDING;

  /* Arrange windows in grid */
  guint index = 0;
  for (GList *l = windows; l != nullptr; l = l->next, index++)
    {
      TerminalWindow *window = TERMINAL_WINDOW (l->data);

      guint row = index / cols;
      guint col = index % cols;

      TerminalWindowGeometry geom;
      geom.x = available_area.x + WINDOW_PADDING + (col * (window_width + WINDOW_PADDING));
      geom.y = available_area.y + WINDOW_PADDING + (row * (window_height + WINDOW_PADDING));
      geom.width = window_width;
      geom.height = window_height;

      terminal_window_layout_apply_geometry (self, window, &geom);
    }
}
