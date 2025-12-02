#ifndef DEFAULTS_HPP_4DCC4E4E_EE4D_4E4F_AA4C_24BD491651B6
#define DEFAULTS_HPP_4DCC4E4E_EE4D_4E4F_AA4C_24BD491651B6

#include <chrono>

/* Select default clock
 *
 * The steady clock is a reasonable choice. It is monotonic, meaning that time
 * will never seem to go backwards (this can happen, e.g., during daylight
 * savings switches, or due to adjustments from something like NTP).
 *
 * std::chrono::high_resolution_clock is a reasonable alternative.
 */
using Clock = std::chrono::steady_clock;

/* Alias: time duration in seconds, as a float.
 *
 * Don't use for long durations (e.g., time since application start or
 * similar), as float accuracy decreases quickly.
 */
using Secondsf = std::chrono::duration<float, std::ratio<1>>;

#endif // DEFAULTS_HPP_4DCC4E4E_EE4D_4E4F_AA4C_24BD491651B6
