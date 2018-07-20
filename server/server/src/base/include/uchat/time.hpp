/* This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#pragma once
#include <time.h>
#include <string>
namespace uchat
{
/**
 * Get current CLOCK_MONOTONIC time in seconds
 * CLOCK_MONOTONIC: 从系统启动这一刻起开始计时 不受系统时间被用户改变的影响
 * @return CLOCK_MONOTONIC time in seconds when success else 0.0
 * @note but is affected by the incremental adjustments performed by
 * adjtime(3) and NTP
 */
extern double NowSecond() noexcept;
/**
 * @details
 * CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific)
 * Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based
 * time that is not subject to NTP adjustments or the incremental adjustments
 * performed by adjtime(3)
 * @note diff CLOCK_BOOTTIME but NOT includes any time that the system is
 * suspended
 */
extern double NowRawSecond() noexcept;
extern double FasterNowSecond() noexcept;
/**
 * Get current CLOCK_MONOTONIC time in nanoseconds
 * @return CLOCK_MONOTONIC time in nanoseconds when success else 0.0
 */
extern double NowNanosecond() noexcept;
extern double FasterNowNanosecond() noexcept;
/**
 * Get current CLOCK_REALTIME time in seconds
 * CLOCK_REALTIME: 系统实时时间 随系统实时时间改变而改变
 * 即从 UTC 1970-1-1 0:0:0 开始计时
 * 中间时刻如果系统时间被用户改成其他 则对应的时间相应改变
 * @return CLOCK_REALTIME time in seconds when success else 0.0
 */
extern double NowSecondRealTime() noexcept;
extern double FasterNowSecondRealTime() noexcept;
/**
 * Check if is timedout
 * @param deadline is beginning NowSecond() + timeout-seconds,
 * if < 0 always false
 * @return true is timedout, else false
 */
extern bool IsTimeout(double const deadline) noexcept;
#if !defined _WIN32 || !_WIN32
extern double LocalTimezone(struct tm const& localctm) noexcept;
#endif
extern bool UtcSecToLocalCtm(time_t const utcSec, struct tm& localctm) noexcept;
/// Make current and local Xx yyyy-MM-dd hh:mm:ss.nnnnnnnnn
extern std::string MakeTzLocalNanoTimestamp() noexcept;
/// Make current and local Xx yy-MM-dd hh:mm:ss.nnnnnnnnn
extern std::string MakeShortTzLocalNanoTimestamp() noexcept;
}
