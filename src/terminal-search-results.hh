/*
 * terminal-search-results.hh
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

G_BEGIN_DECLS

/**
 * TerminalSearchResult:
 *
 * A single search result with context information.
 */
typedef struct {
  char *text;        /* The matched text */
  gint row;          /* Row number in terminal */
  gint column;       /* Column number in terminal */
  char *context;     /* Context around the match */
} TerminalSearchResult;

/**
 * TerminalSearchResults:
 *
 * Displays search results in a listbox with context preview.
 * Shows "X of Y matches" counter and navigation.
 */

#define TERMINAL_TYPE_SEARCH_RESULTS (terminal_search_results_get_type ())
#define TERMINAL_SEARCH_RESULTS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TERMINAL_TYPE_SEARCH_RESULTS, TerminalSearchResults))
#define TERMINAL_IS_SEARCH_RESULTS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TERMINAL_TYPE_SEARCH_RESULTS))

typedef struct _TerminalSearchResults TerminalSearchResults;
typedef struct {
  GtkBoxClass parent_class;
} TerminalSearchResultsClass;

GType terminal_search_results_get_type (void);

/**
 * TerminalSearchResults methods:
 */

GtkWidget *terminal_search_results_new (void);

void terminal_search_results_set_match_count (TerminalSearchResults *self,
                                             guint current,
                                             guint total);

void terminal_search_results_add_result (TerminalSearchResults *self,
                                        const TerminalSearchResult *result);

void terminal_search_results_clear (TerminalSearchResults *self);

void terminal_search_results_highlight_current (TerminalSearchResults *self,
                                               guint index);

G_END_DECLS
