/*
 * terminal-buffer-search.hh
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

#include <vte/vte.h>
#include "terminal-search-results.hh"

G_BEGIN_DECLS

/**
 * terminal_buffer_search_scan:
 * @terminal: VteTerminal to scan
 * @pattern: PCRE2 regex pattern string
 * @pcre2_flags: PCRE2 compilation flags
 * @max_rows: Maximum number of rows to scan from the bottom
 * @max_results: Maximum number of results to return
 *
 * Scans the terminal buffer for regex matches and returns detailed results.
 * The caller must free the returned list via terminal_buffer_search_free_results().
 *
 * Warning: This function calls VTE APIs and is NOT thread-safe.
 * It must only be called from the main (UI) thread.
 * For background thread usage, use terminal_buffer_search_scan_text() instead.
 *
 * Returns: (transfer full): GList of TerminalSearchResult* or NULL on error
 */
GList *terminal_buffer_search_scan (VteTerminal *terminal,
                                    const char  *pattern,
                                    guint32      pcre2_flags,
                                    glong        max_rows,
                                    guint        max_results);

/**
 * terminal_buffer_search_scan_text:
 * @buffer_text: Pre-extracted buffer text (newline-separated rows)
 * @cols: Number of columns per row
 * @pattern: PCRE2 regex pattern string
 * @pcre2_flags: PCRE2 compilation flags
 * @max_results: Maximum number of results to return
 *
 * Thread-safe alternative to terminal_buffer_search_scan().
 * Scans pre-extracted text for regex matches without accessing VTE widgets.
 * Call from any thread. The caller must free the returned list via
 * terminal_buffer_search_free_results().
 *
 * Returns: (transfer full): GList of TerminalSearchResult* or NULL on error
 */
GList *terminal_buffer_search_scan_text (const char  *buffer_text,
                                         glong        cols,
                                         const char  *pattern,
                                         guint32      pcre2_flags,
                                         guint        max_results);

/**
 * terminal_buffer_search_free_results:
 * @results: GList returned by terminal_buffer_search_scan()
 *
 * Frees a result list and all TerminalSearchResult structs within.
 */
void terminal_buffer_search_free_results (GList *results);

/**
 * terminal_buffer_search_count_matches:
 * @terminal: VteTerminal to scan
 * @pattern: PCRE2 regex pattern string
 * @pcre2_flags: PCRE2 compilation flags
 * @max_rows: Maximum number of rows to scan from the bottom
 *
 * Counts matches in the terminal buffer without extracting full context.
 * This is faster than terminal_buffer_search_scan() when only counts are needed.
 *
 * Warning: This function calls VTE APIs and is NOT thread-safe.
 * It must only be called from the main (UI) thread.
 *
 * Returns: Number of matches found (0 on error or no matches)
 */
guint terminal_buffer_search_count_matches (VteTerminal *terminal,
                                            const char  *pattern,
                                            guint32      pcre2_flags,
                                            glong        max_rows);

G_END_DECLS
