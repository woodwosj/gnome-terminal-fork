/*
 * terminal-window-stack.hh
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
 * TerminalWindowStack:
 *
 * Manager for terminal window z-order (stacking) operations.
 * Provides bring-to-front and send-to-back functionality for window management.
 */

#define TERMINAL_TYPE_WINDOW_STACK (terminal_window_stack_get_type ())
#define TERMINAL_WINDOW_STACK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TERMINAL_TYPE_WINDOW_STACK, TerminalWindowStack))
#define TERMINAL_IS_WINDOW_STACK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TERMINAL_TYPE_WINDOW_STACK))

typedef struct _TerminalWindowStack TerminalWindowStack;
typedef struct {
  GObjectClass parent_class;
} TerminalWindowStackClass;

GType terminal_window_stack_get_type (void);

typedef enum {
  TERMINAL_WINDOW_STACK_BRING_TO_FRONT,
  TERMINAL_WINDOW_STACK_SEND_TO_BACK,
} TerminalWindowStackAction;

/**
 * TerminalWindowStack methods:
 */

TerminalWindowStack *terminal_window_stack_new (void);

void terminal_window_stack_add_window (TerminalWindowStack *self,
                                       TerminalWindow *window);

void terminal_window_stack_remove_window (TerminalWindowStack *self,
                                         TerminalWindow *window);

void terminal_window_stack_bring_to_front (TerminalWindowStack *self,
                                          TerminalWindow *window);

void terminal_window_stack_send_to_back (TerminalWindowStack *self,
                                        TerminalWindow *window);

guint terminal_window_stack_get_position (TerminalWindowStack *self,
                                         TerminalWindow *window);

guint terminal_window_stack_get_count (TerminalWindowStack *self);

TerminalWindow *terminal_window_stack_get_nth (TerminalWindowStack *self,
                                               guint index);

G_END_DECLS
