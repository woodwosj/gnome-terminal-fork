/*
 * terminal-search-history.cc
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

#include "terminal-search-history.hh"

#include <glib/gi18n.h>

/* GSettings schema is provided via constructor */
#define TERMINAL_SEARCH_HISTORY_KEY "search-history"

struct _TerminalSearchHistory {
  GObject parent_instance;

  GSettings *settings;
  GStrv history;
};

G_DEFINE_TYPE (TerminalSearchHistory, terminal_search_history, G_TYPE_OBJECT)

/* Properties */
enum {
  PROP_0,
  PROP_SETTINGS,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* ---- Initialization ---- */

static void
terminal_search_history_finalize (GObject *object)
{
  TerminalSearchHistory *self = TERMINAL_SEARCH_HISTORY (object);

  g_clear_object (&self->settings);
  g_strfreev (self->history);

  G_OBJECT_CLASS (terminal_search_history_parent_class)->finalize (object);
}

static void
terminal_search_history_set_property (GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
  TerminalSearchHistory *self = TERMINAL_SEARCH_HISTORY (object);

  switch (prop_id)
    {
    case PROP_SETTINGS:
      self->settings = (GSettings *)g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
terminal_search_history_get_property (GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
  TerminalSearchHistory *self = TERMINAL_SEARCH_HISTORY (object);

  switch (prop_id)
    {
    case PROP_SETTINGS:
      g_value_set_object (value, self->settings);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
terminal_search_history_load (TerminalSearchHistory *self)
{
  g_autoptr(GVariant) variant = nullptr;

  if (!self->settings)
    return;

  variant = g_settings_get_value (self->settings, TERMINAL_SEARCH_HISTORY_KEY);
  if (variant)
    self->history = g_variant_dup_strv (variant, nullptr);
  else
    self->history = g_new0 (char *, 1);
}

static void
terminal_search_history_class_init (TerminalSearchHistoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = terminal_search_history_finalize;
  object_class->set_property = terminal_search_history_set_property;
  object_class->get_property = terminal_search_history_get_property;

  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings", nullptr, nullptr,
                         G_TYPE_SETTINGS,
                         GParamFlags(G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
terminal_search_history_init (TerminalSearchHistory *self)
{
  self->history = nullptr;
}

/* ---- Public API ---- */

/**
 * terminal_search_history_new:
 * @settings: (nullable): a #GSettings for persistence
 *
 * Creates a new search history manager.
 *
 * Returns: (transfer full): a new #TerminalSearchHistory
 */
TerminalSearchHistory *
terminal_search_history_new (GSettings *settings)
{
  TerminalSearchHistory *self = (TerminalSearchHistory *)g_object_new (TERMINAL_TYPE_SEARCH_HISTORY,
                                             "settings", settings,
                                             nullptr);
  terminal_search_history_load (self);
  return self;
}

/**
 * terminal_search_history_add:
 * @self: a #TerminalSearchHistory
 * @query: the search query
 *
 * Adds a search query to the history.
 * If already present, moves it to the top.
 */
void
terminal_search_history_add (TerminalSearchHistory *self,
                             const char *query)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_HISTORY (self));
  g_return_if_fail (query != nullptr && *query != '\0');

  /* Remove if already exists */
  terminal_search_history_remove (self, query);

  /* Create new array with query at front */
  gsize len = g_strv_length (self->history);
  gsize new_len = MIN (len + 1, TERMINAL_SEARCH_HISTORY_MAX_ITEMS);

  char **new_history = g_new0 (char *, new_len + 1);
  new_history[0] = g_strdup (query);

  for (gsize i = 0; i < new_len - 1 && i < len; i++)
    new_history[i + 1] = g_strdup (self->history[i]);

  g_strfreev (self->history);
  self->history = new_history;

  /* Save to GSettings */
  if (self->settings)
    {
      g_autoptr(GVariant) variant = g_variant_new_strv ((const char **)self->history, -1);
      g_settings_set_value (self->settings, TERMINAL_SEARCH_HISTORY_KEY, variant);
    }
}

/**
 * terminal_search_history_get_all:
 * @self: a #TerminalSearchHistory
 *
 * Gets all search queries in the history.
 *
 * Returns: (transfer full) (array zero-terminated=1): array of queries
 */
char **
terminal_search_history_get_all (TerminalSearchHistory *self)
{
  g_return_val_if_fail (TERMINAL_IS_SEARCH_HISTORY (self), nullptr);

  return g_strdupv (self->history);
}

/**
 * terminal_search_history_get_recent:
 * @self: a #TerminalSearchHistory
 *
 * Gets the most recent search query.
 *
 * Returns: (transfer none) (nullable): the most recent query, or %NULL
 */
char *
terminal_search_history_get_recent (TerminalSearchHistory *self)
{
  g_return_val_if_fail (TERMINAL_IS_SEARCH_HISTORY (self), nullptr);

  return self->history && self->history[0] ? self->history[0] : nullptr;
}

/**
 * terminal_search_history_clear:
 * @self: a #TerminalSearchHistory
 *
 * Clears all search history.
 */
void
terminal_search_history_clear (TerminalSearchHistory *self)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_HISTORY (self));

  g_strfreev (self->history);
  self->history = g_new0 (char *, 1);

  if (self->settings)
    {
      g_autoptr(GVariant) variant = g_variant_new_strv ((const char **)self->history, -1);
      g_settings_set_value (self->settings, TERMINAL_SEARCH_HISTORY_KEY, variant);
    }
}

/**
 * terminal_search_history_contains:
 * @self: a #TerminalSearchHistory
 * @query: the query to check
 *
 * Checks if a query exists in history.
 *
 * Returns: %TRUE if query exists
 */
gboolean
terminal_search_history_contains (TerminalSearchHistory *self,
                                  const char *query)
{
  g_return_val_if_fail (TERMINAL_IS_SEARCH_HISTORY (self), FALSE);
  g_return_val_if_fail (query != nullptr, FALSE);

  return g_strv_contains ((const char * const *)self->history, query);
}

/**
 * terminal_search_history_remove:
 * @self: a #TerminalSearchHistory
 * @query: the query to remove
 *
 * Removes a query from the history.
 */
void
terminal_search_history_remove (TerminalSearchHistory *self,
                                const char *query)
{
  g_return_if_fail (TERMINAL_IS_SEARCH_HISTORY (self));
  g_return_if_fail (query != nullptr);

  gsize len = g_strv_length (self->history);
  gint pos = -1;

  for (gsize i = 0; i < len; i++)
    {
      if (g_strcmp0 (self->history[i], query) == 0)
        {
          pos = i;
          break;
        }
    }

  if (pos < 0)
    return;

  /* Shift remaining items */
  for (gsize i = pos; i < len - 1; i++)
    self->history[i] = self->history[i + 1];
  self->history[len - 1] = nullptr;

  /* Save to GSettings */
  if (self->settings)
    {
      g_autoptr(GVariant) variant = g_variant_new_strv ((const char **)self->history, -1);
      g_settings_set_value (self->settings, TERMINAL_SEARCH_HISTORY_KEY, variant);
    }
}
