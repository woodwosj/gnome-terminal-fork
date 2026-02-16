/*
 * terminal-search-history.hh
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
#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * TerminalSearchHistory:
 *
 * Manages search history with persistence via GSettings.
 * Stores recent search queries for quick access and reuse.
 */

#define TERMINAL_TYPE_SEARCH_HISTORY (terminal_search_history_get_type ())
#define TERMINAL_SEARCH_HISTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TERMINAL_TYPE_SEARCH_HISTORY, TerminalSearchHistory))
#define TERMINAL_IS_SEARCH_HISTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TERMINAL_TYPE_SEARCH_HISTORY))

typedef struct _TerminalSearchHistory TerminalSearchHistory;
typedef struct {
  GObjectClass parent_class;
} TerminalSearchHistoryClass;

GType terminal_search_history_get_type (void);

#define TERMINAL_SEARCH_HISTORY_MAX_ITEMS 20

/**
 * TerminalSearchHistory methods:
 */

TerminalSearchHistory *terminal_search_history_new (GSettings *settings);

void terminal_search_history_add (TerminalSearchHistory *self,
                                 const char *query);

char **terminal_search_history_get_all (TerminalSearchHistory *self);

char *terminal_search_history_get_recent (TerminalSearchHistory *self);

void terminal_search_history_clear (TerminalSearchHistory *self);

gboolean terminal_search_history_contains (TerminalSearchHistory *self,
                                          const char *query);

void terminal_search_history_remove (TerminalSearchHistory *self,
                                    const char *query);

G_END_DECLS
