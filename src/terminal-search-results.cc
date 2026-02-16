/*
 * terminal-search-results.cc
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

#include "terminal-search-results.hh"

#include <glib/gi18n.h>

struct _TerminalSearchResults {
  GtkBox parent_instance;

  GtkLabel *match_counter;
  GtkListBox *results_list;
  GtkScrolledWindow *scrolled_window;
  GList *results;
};

G_DEFINE_TYPE (TerminalSearchResults, terminal_search_results, GTK_TYPE_BOX)

/* ---- Initialization ---- */

static void
terminal_search_results_finalize (GObject *object)
{
  TerminalSearchResults *self = TERMINAL_SEARCH_RESULTS (object);

  g_list_free_full (self->results, (GDestroyNotify)g_free);

  G_OBJECT_CLASS (terminal_search_results_parent_class)->finalize (object);
}

static void
terminal_search_results_class_init (TerminalSearchResultsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = terminal_search_results_finalize;
}

static void
terminal_search_results_init (TerminalSearchResults *self)
{
  /* GTK3 setup: labels and containers initialized with defaults */
  /* Match counter label */
  self->match_counter = GTK_LABEL (gtk_label_new (_("No results")));
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (self->match_counter), FALSE, FALSE, 0);

  /* Scrolled window for results list */
  self->scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (nullptr, nullptr));
  gtk_scrolled_window_set_policy (self->scrolled_window,
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (GTK_WIDGET (self->scrolled_window), 300, 200);
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (self->scrolled_window), TRUE, TRUE, 0);

  /* Results listbox */
  self->results_list = GTK_LIST_BOX (gtk_list_box_new ());
  gtk_list_box_set_selection_mode (self->results_list, GTK_SELECTION_SINGLE);
  gtk_container_add (GTK_CONTAINER (self->scrolled_window), GTK_WIDGET (self->results_list));

  self->results = nullptr;
}

/* ---- Public API ---- */

/**
 * terminal_search_results_new:
 *
 * Creates a new search results display widget.
 *
 * Returns: (transfer full): a new #TerminalSearchResults widget
 */
GtkWidget *
terminal_search_results_new (void)
{
  return (GtkWidget *)g_object_new (TERMINAL_TYPE_SEARCH_RESULTS, nullptr);
}

/**
 * terminal_search_results_set_match_count:
 * @self: a #TerminalSearchResults
 * @current: current match number (1-based)
 * @total: total number of matches
 *
 * Updates the "X of Y matches" counter display.
 */
void
terminal_search_results_set_match_count (TerminalSearchResults *self,
                                        guint current,
                                        guint total)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_RESULTS (self));

  if (total == 0)
    {
      gtk_label_set_text (self->match_counter, _("No results"));
    }
  else
    {
      g_autofree char *text = g_strdup_printf (
        ngettext ("Match %u of %u", "Match %u of %u", total),
        current, total);
      gtk_label_set_text (self->match_counter, text);
    }
}

/**
 * terminal_search_results_add_result:
 * @self: a #TerminalSearchResults
 * @result: a #TerminalSearchResult
 *
 * Adds a search result to the display.
 */
void
terminal_search_results_add_result (TerminalSearchResults *self,
                                   const TerminalSearchResult *result)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_RESULTS (self));
  g_return_if_fail (result != nullptr);

  /* Create result item */
  GtkBox *box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 3));
  gtk_widget_set_margin_top (GTK_WIDGET (box), 6);
  gtk_widget_set_margin_bottom (GTK_WIDGET (box), 6);
  gtk_widget_set_margin_start (GTK_WIDGET (box), 6);
  gtk_widget_set_margin_end (GTK_WIDGET (box), 6);

  /* Result text */
  GtkLabel *text_label = GTK_LABEL (gtk_label_new (result->text));
  gtk_label_set_ellipsize (text_label, PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars (text_label, 50);
  gtk_box_pack_start (box, GTK_WIDGET (text_label), FALSE, FALSE, 0);

  /* Context */
  if (result->context && *result->context != '\0')
    {
      GtkLabel *context_label = GTK_LABEL (gtk_label_new (result->context));
      gtk_label_set_ellipsize (context_label, PANGO_ELLIPSIZE_END);
      gtk_label_set_max_width_chars (context_label, 50);
      gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (context_label)), "dim-label");
      gtk_box_pack_start (box, GTK_WIDGET (context_label), FALSE, FALSE, 0);
    }

  /* Add to listbox */
  gtk_list_box_insert (self->results_list, GTK_WIDGET (box), -1);
  gtk_widget_show_all (GTK_WIDGET (box));

  /* Keep reference to the result data */
  TerminalSearchResult *result_copy = g_new (TerminalSearchResult, 1);
  result_copy->text = g_strdup (result->text);
  result_copy->row = result->row;
  result_copy->column = result->column;
  result_copy->context = g_strdup (result->context);
  self->results = g_list_append (self->results, result_copy);
}

/**
 * terminal_search_results_clear:
 * @self: a #TerminalSearchResults
 *
 * Clears all results from the display.
 */
void
terminal_search_results_clear (TerminalSearchResults *self)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_RESULTS (self));

  /* Clear listbox */
  GList *children = gtk_container_get_children (GTK_CONTAINER (self->results_list));
  for (GList *l = children; l != nullptr; l = l->next)
    {
      gtk_container_remove (GTK_CONTAINER (self->results_list), GTK_WIDGET (l->data));
    }
  g_list_free (children);

  /* Clear results data */
  g_list_free_full (self->results, (GDestroyNotify)g_free);
  self->results = nullptr;

  /* Reset counter */
  gtk_label_set_text (self->match_counter, _("No results"));
}

/**
 * terminal_search_results_highlight_current:
 * @self: a #TerminalSearchResults
 * @index: index of result to highlight
 *
 * Highlights a specific result in the list.
 */
void
terminal_search_results_highlight_current (TerminalSearchResults *self,
                                          guint index)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_RESULTS (self));

  GtkListBoxRow *row = gtk_list_box_get_row_at_index (self->results_list, index);
  if (row)
    {
      gtk_list_box_select_row (self->results_list, row);
      gtk_widget_grab_focus (GTK_WIDGET (row));
    }
}
