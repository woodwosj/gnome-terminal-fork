/*
 * terminal-buffer-search.cc
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

#include "terminal-buffer-search.hh"
#include "terminal-pcre2.hh"

#include <glib.h>
#include <string.h>

/**
 * terminal_buffer_search_scan:
 *
 * Scans the VTE terminal buffer for regex matches using PCRE2.
 * Extracts matched text and surrounding context for display in results panel.
 */
GList *
terminal_buffer_search_scan (VteTerminal *terminal,
                             const char  *pattern,
                             guint32      pcre2_flags,
                             glong        max_rows,
                             guint        max_results)
{
  g_return_val_if_fail (VTE_IS_TERMINAL (terminal), NULL);
  g_return_val_if_fail (pattern != NULL, NULL);

  GList *results = NULL;
  glong total_rows = vte_terminal_get_row_count (terminal);
  glong cols = vte_terminal_get_column_count (terminal);
  glong start_row = MAX (0, total_rows - max_rows);

  /* Compile PCRE2 pattern */
  int errcode;
  PCRE2_SIZE erroffset;
  pcre2_code_8 *re = pcre2_compile_8 (
    (PCRE2_SPTR8) pattern,
    PCRE2_ZERO_TERMINATED,
    pcre2_flags,
    &errcode,
    &erroffset,
    NULL);

  if (!re)
    return NULL;

  pcre2_match_data_8 *match_data = pcre2_match_data_create_from_pattern_8 (re, NULL);
  guint result_count = 0;

  /* Scan row by row */
  for (glong row = start_row; row < total_rows && result_count < max_results; row++) {
    gsize length = 0;
    char *line = vte_terminal_get_text_range_format (
      terminal,
      VTE_FORMAT_TEXT,
      row, 0,
      row, cols - 1,
      &length);

    if (!line || length == 0) {
      g_free (line);
      continue;
    }

    /* Find all matches in this line */
    PCRE2_SIZE offset = 0;
    while (offset < length) {
      int rc = pcre2_match_8 (
        re,
        (PCRE2_SPTR8) line,
        length,
        offset,
        0,
        match_data,
        NULL);

      if (rc < 1)
        break;

      PCRE2_SIZE *ovector = pcre2_get_ovector_pointer_8 (match_data);
      PCRE2_SIZE match_start = ovector[0];
      PCRE2_SIZE match_end = ovector[1];

      /* Build result */
      TerminalSearchResult *result = g_new0 (TerminalSearchResult, 1);
      result->row = (gint) row;
      result->column = (gint) match_start;
      result->text = g_strndup (line + match_start, match_end - match_start);

      /* Extract context (20 chars before/after) */
      glong ctx_start = MAX (0, (glong) match_start - 20);
      glong ctx_end = MIN ((glong) length, (glong) match_end + 20);
      result->context = g_strndup (line + ctx_start, ctx_end - ctx_start);

      results = g_list_prepend (results, result);
      result_count++;

      if (result_count >= max_results)
        break;

      offset = match_end; /* Advance past this match */
    }

    g_free (line);
  }

  pcre2_match_data_free_8 (match_data);
  pcre2_code_free_8 (re);

  return g_list_reverse (results); /* Return in chronological order */
}

/**
 * terminal_buffer_search_scan_text:
 *
 * Scans pre-extracted buffer text for regex matches using PCRE2.
 * This function is thread-safe as it does not access any GTK/VTE widgets.
 *
 * @buffer_text: Pre-extracted text from VTE terminal buffer
 * @cols: Number of columns per row (for row/column calculation)
 * @pattern: PCRE2 regex pattern string
 * @pcre2_flags: PCRE2 compilation flags
 * @max_results: Maximum number of results to return
 *
 * Returns: (transfer full): GList of TerminalSearchResult* or NULL
 */
GList *
terminal_buffer_search_scan_text (const char  *buffer_text,
                                  glong        cols,
                                  const char  *pattern,
                                  guint32      pcre2_flags,
                                  guint        max_results)
{
  g_return_val_if_fail (buffer_text != NULL, NULL);
  g_return_val_if_fail (pattern != NULL, NULL);

  GList *results = NULL;

  /* Compile PCRE2 pattern */
  int errcode;
  PCRE2_SIZE erroffset;
  pcre2_code_8 *re = pcre2_compile_8 (
    (PCRE2_SPTR8) pattern,
    PCRE2_ZERO_TERMINATED,
    pcre2_flags,
    &errcode,
    &erroffset,
    NULL);

  if (!re)
    return NULL;

  pcre2_match_data_8 *match_data = pcre2_match_data_create_from_pattern_8 (re, NULL);
  guint result_count = 0;
  gsize text_len = strlen (buffer_text);

  /* Split text into lines and process */
  const char *line_start = buffer_text;
  glong current_row = 0;

  while (line_start < buffer_text + text_len && result_count < max_results) {
    const char *line_end = strchr (line_start, '\n');
    gsize line_len;
    if (line_end)
      line_len = line_end - line_start;
    else
      line_len = (buffer_text + text_len) - line_start;

    if (line_len > 0) {
      /* Find all matches in this line */
      PCRE2_SIZE offset = 0;
      while (offset < line_len && result_count < max_results) {
        int rc = pcre2_match_8 (
          re,
          (PCRE2_SPTR8) line_start,
          line_len,
          offset,
          0,
          match_data,
          NULL);

        if (rc < 1)
          break;

        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer_8 (match_data);
        PCRE2_SIZE match_start = ovector[0];
        PCRE2_SIZE match_end = ovector[1];

        /* Build result */
        TerminalSearchResult *result = g_new0 (TerminalSearchResult, 1);
        result->row = (gint) current_row;
        result->column = (gint) match_start;
        result->text = g_strndup (line_start + match_start, match_end - match_start);

        /* Extract context (20 chars before/after) */
        glong ctx_start = MAX (0, (glong) match_start - 20);
        glong ctx_end = MIN ((glong) line_len, (glong) match_end + 20);
        result->context = g_strndup (line_start + ctx_start, ctx_end - ctx_start);

        results = g_list_prepend (results, result);
        result_count++;

        offset = match_end > offset ? match_end : offset + 1;
      }
    }

    current_row++;
    if (line_end)
      line_start = line_end + 1;
    else
      break;
  }

  pcre2_match_data_free_8 (match_data);
  pcre2_code_free_8 (re);

  return g_list_reverse (results);
}

/**
 * terminal_buffer_search_free_results:
 *
 * Frees a GList of TerminalSearchResult structs.
 */
void
terminal_buffer_search_free_results (GList *results)
{
  for (GList *l = results; l; l = l->next) {
    TerminalSearchResult *result = (TerminalSearchResult *) l->data;
    g_free (result->text);
    g_free (result->context);
    g_free (result);
  }
  g_list_free (results);
}

/**
 * terminal_buffer_search_count_matches:
 *
 * Counts matches in the terminal buffer without extracting full context.
 * Faster than terminal_buffer_search_scan() for match count badges.
 */
guint
terminal_buffer_search_count_matches (VteTerminal *terminal,
                                      const char  *pattern,
                                      guint32      pcre2_flags,
                                      glong        max_rows)
{
  g_return_val_if_fail (VTE_IS_TERMINAL (terminal), 0);
  g_return_val_if_fail (pattern != NULL, 0);

  glong total_rows = vte_terminal_get_row_count (terminal);
  glong cols = vte_terminal_get_column_count (terminal);
  glong start_row = MAX (0, total_rows - max_rows);

  /* Compile PCRE2 pattern */
  int errcode;
  PCRE2_SIZE erroffset;
  pcre2_code_8 *re = pcre2_compile_8 (
    (PCRE2_SPTR8) pattern,
    PCRE2_ZERO_TERMINATED,
    pcre2_flags,
    &errcode,
    &erroffset,
    NULL);

  if (!re)
    return 0;

  pcre2_match_data_8 *match_data = pcre2_match_data_create_from_pattern_8 (re, NULL);
  guint match_count = 0;

  /* Scan row by row */
  for (glong row = start_row; row < total_rows; row++) {
    gsize length = 0;
    char *line = vte_terminal_get_text_range_format (
      terminal,
      VTE_FORMAT_TEXT,
      row, 0,
      row, cols - 1,
      &length);

    if (!line || length == 0) {
      g_free (line);
      continue;
    }

    /* Count all matches in this line */
    PCRE2_SIZE offset = 0;
    while (offset < length) {
      int rc = pcre2_match_8 (
        re,
        (PCRE2_SPTR8) line,
        length,
        offset,
        0,
        match_data,
        NULL);

      if (rc < 1)
        break;

      PCRE2_SIZE *ovector = pcre2_get_ovector_pointer_8 (match_data);
      match_count++;
      offset = ovector[1]; /* Advance past this match */
    }

    g_free (line);
  }

  pcre2_match_data_free_8 (match_data);
  pcre2_code_free_8 (re);

  return match_count;
}
