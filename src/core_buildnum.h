#ifndef BUILDNUM_H
#define BUILDNUM_H

/**
 * editor_build_date - Compilation date string
 *
 * Contains the date when the editor was compiled in format "MMM DD YYYY"
 * (e.g., "Nov 12 2025"). Set by the __DATE__ compiler macro.
 */
extern const char *editor_build_date;

/**
 * editor_build_time - Compilation time string
 *
 * Contains the time when the editor was compiled in format "HH:MM:SS"
 * (e.g., "14:30:45"). Set by the __TIME__ compiler macro.
 */
extern const char *editor_build_time;

/**
 * editorGetBuildNumber - Get the build number
 *
 * Returns the number of days elapsed since the reference date
 * (Sep 13 2020 - holoEN Myth debut date). This provides a simple
 * incrementing build number based on compilation date.
 *
 * The function uses lazy evaluation - it calculates the build number
 * only once on first call and caches the result.
 *
 * Returns: Number of days since Sep 13 2020
 *
 * Example:
 *   If compiled on Sep 13 2020: returns 0
 *   If compiled on Sep 14 2020: returns 1
 *   If compiled on Sep 13 2021: returns 365
 */
int editorGetBuildNumber(void);

#endif