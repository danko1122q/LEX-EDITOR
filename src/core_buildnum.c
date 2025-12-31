#include "core_buildnum.h"

// Build date and time from compiler macros
const char *editor_build_date = __DATE__;
const char *editor_build_time = __TIME__;

// Current compilation date (set by __DATE__ macro)
static const char *date = __DATE__;

// Month name abbreviations for date parsing
static const char *mon[12]  = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Number of days in each month (non-leap year)
static char        mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Flag to track if build number has been computed
static bool computed = false;

// Cached build number (days since reference date)
static int  build_number;

/**
 * computeBuildNumber - Calculate build number from compilation date
 * 
 * Computes the number of days since the reference date (Sep 13 2020).
 * The build number accounts for leap years and is calculated using
 * the __DATE__ macro which provides the compilation date.
 * 
 * Reference date: Sep 13 2020 (holoEN Myth debut date)
 */
static void computeBuildNumber(void)
{
  int m = 0;  // Month index
  int d = 0;  // Day of year
  int y = 0;  // Year offset from 1900

  // Find which month the build date is in
  for (m = 0; m < 11; m++)
  {
    // Compare first 3 characters with month names
    if (strncmp(&date[0], mon[m], 3) == 0)
      break;
    
    // Add days from previous months
    d += mond[m];
  }

  // Add the day of the month (minus 1 for zero-based counting)
  d += atoi(&date[4]) - 1;

  // Extract year and convert to offset from 1900
  y = atoi(&date[7]) - 1900;

  // Calculate total days: day of year + (years * 365.25 to account for leap years)
  build_number = d + (int) ((y - 1) * 365.25);

  // Adjust for leap year if current year is leap and we're past February
  if (((y % 4) == 0) && m > 1)
  {
    build_number += 1;
  }

  // Subtract reference date offset (Sep 13 2020)
  // This makes build number 0 = Sep 13 2020
  build_number -= 43720;
}

/**
 * editorGetBuildNumber - Get the build number
 * 
 * Returns the number of days since Sep 13 2020 (holoEN Myth debut date).
 * The computation is lazy - it only calculates once on first call
 * and caches the result for subsequent calls.
 * 
 * Returns: Number of days since Sep 13 2020
 */
int editorGetBuildNumber(void)
{
  // Compute build number on first call only
  if (!computed)
  {
    computeBuildNumber();
    computed = true;
  }
  
  return build_number;
}