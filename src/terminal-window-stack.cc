/*
 * terminal-window-stack.cc
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

#include "terminal-window-stack.hh"

#include <glib/gi18n.h>

struct _TerminalWindowStack {
  GObject parent_instance;

  /* List of TerminalWindow* ordered from back to front */
  GList *windows;
};

G_DEFINE_TYPE (TerminalWindowStack, terminal_window_stack, G_TYPE_OBJECT)

/* ---- Initialization ---- */

static void
terminal_window_stack_finalize (GObject *object)
{
  TerminalWindowStack *self = TERMINAL_WINDOW_STACK (object);

  g_list_free (self->windows);

  G_OBJECT_CLASS (terminal_window_stack_parent_class)->finalize (object);
}

static void
terminal_window_stack_class_init (TerminalWindowStackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = terminal_window_stack_finalize;
}

static void
terminal_window_stack_init (TerminalWindowStack *self)
{
  self->windows = nullptr;
}

/* ---- Public API ---- */

/**
 * terminal_window_stack_new:
 *
 * Creates a new window stack manager.
 *
 * Returns: (transfer full): a new #TerminalWindowStack
 */
TerminalWindowStack *
terminal_window_stack_new (void)
{
  return (TerminalWindowStack *)g_object_new (TERMINAL_TYPE_WINDOW_STACK, nullptr);
}

/**
 * terminal_window_stack_add_window:
 * @self: a #TerminalWindowStack
 * @window: a #TerminalWindow
 *
 * Adds a window to the stack at the top (front position).
 */
void
terminal_window_stack_add_window (TerminalWindowStack *self,
                                  TerminalWindow *window)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_STACK (self));
  g_return_if_fail (TERMINAL_IS_WINDOW (window));

  /* Add to the front of the list (top of stack) */
  self->windows = g_list_prepend (self->windows, window);
}

/**
 * terminal_window_stack_remove_window:
 * @self: a #TerminalWindowStack
 * @window: a #TerminalWindow
 *
 * Removes a window from the stack.
 */
void
terminal_window_stack_remove_window (TerminalWindowStack *self,
                                     TerminalWindow *window)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_STACK (self));
  g_return_if_fail (TERMINAL_IS_WINDOW (window));

  self->windows = g_list_remove (self->windows, window);
}

/**
 * terminal_window_stack_bring_to_front:
 * @self: a #TerminalWindowStack
 * @window: a #TerminalWindow
 *
 * Moves a window to the front (top) of the stack.
 * Also calls gtk_window_present() to raise the window visually.
 */
void
terminal_window_stack_bring_to_front (TerminalWindowStack *self,
                                      TerminalWindow *window)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_STACK (self));
  g_return_if_fail (TERMINAL_IS_WINDOW (window));

  /* Remove from current position */
  self->windows = g_list_remove (self->windows, window);

  /* Add to front */
  self->windows = g_list_prepend (self->windows, window);

  /* Visually raise the window */
  gtk_window_present (GTK_WINDOW (window));
}

/**
 * terminal_window_stack_send_to_back:
 * @self: a #TerminalWindowStack
 * @window: a #TerminalWindow
 *
 * Moves a window to the back (bottom) of the stack.
 */
void
terminal_window_stack_send_to_back (TerminalWindowStack *self,
                                    TerminalWindow *window)
{
  g_return_if_fail (TERMINAL_IS_WINDOW_STACK (self));
  g_return_if_fail (TERMINAL_IS_WINDOW (window));

  /* Remove from current position */
  self->windows = g_list_remove (self->windows, window);

  /* Add to back */
  self->windows = g_list_append (self->windows, window);
}

/**
 * terminal_window_stack_get_position:
 * @self: a #TerminalWindowStack
 * @window: a #TerminalWindow
 *
 * Gets the position of a window in the stack (0 = back, highest = front).
 *
 * Returns: the position, or -1 if window not in stack
 */
guint
terminal_window_stack_get_position (TerminalWindowStack *self,
                                    TerminalWindow *window)
{
  g_return_val_if_fail (TERMINAL_IS_WINDOW_STACK (self), 0);
  g_return_val_if_fail (TERMINAL_IS_WINDOW (window), 0);

  gint pos = g_list_index (self->windows, window);
  return pos >= 0 ? (guint)pos : 0;
}

/**
 * terminal_window_stack_get_count:
 * @self: a #TerminalWindowStack
 *
 * Gets the total number of windows in the stack.
 *
 * Returns: the number of windows
 */
guint
terminal_window_stack_get_count (TerminalWindowStack *self)
{
  g_return_val_if_fail (TERMINAL_IS_WINDOW_STACK (self), 0);

  return g_list_length (self->windows);
}

/**
 * terminal_window_stack_get_nth:
 * @self: a #TerminalWindowStack
 * @index: the index of the window (0 = back, highest = front)
 *
 * Gets the window at the specified position.
 *
 * Returns: (transfer none) (nullable): the window, or %NULL if index out of range
 */
TerminalWindow *
terminal_window_stack_get_nth (TerminalWindowStack *self,
                               guint index)
{
  g_return_val_if_fail (TERMINAL_IS_WINDOW_STACK (self), nullptr);

  GList *link = g_list_nth (self->windows, index);
  return link ? TERMINAL_WINDOW (link->data) : nullptr;
}
