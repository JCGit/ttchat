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
#include "uchat/time.hpp"
#include <time.h>
#include <string.h>
#include <limits>
namespace uchat
{
double NowSecond() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_MONOTONIC, &tp)) {
        return ((static_cast<double>(tp.tv_nsec) / 1e9) + tp.tv_sec);
    } else {
        return 0.0;
    }
}
double NowRawSecond() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_MONOTONIC_RAW, &tp)) {
        return ((static_cast<double>(tp.tv_nsec) / 1e9) + tp.tv_sec);
    } else {
        return 0.0;
    }
}
double FasterNowSecond() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_MONOTONIC_COARSE, &tp)) {
        return ((static_cast<double>(tp.tv_nsec) / 1e9) + tp.tv_sec);
    } else {
        return 0.0;
    }
}
double NowNanosecond() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_MONOTONIC, &tp)) {
        return (tp.tv_nsec + (tp.tv_sec * 1e9));
    } else {
        return 0.0;
    }
}
double FasterNowNanosecond() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_MONOTONIC_COARSE, &tp)) {
        return (tp.tv_nsec + (tp.tv_sec * 1e9));
    } else {
        return 0.0;
    }
}
double NowSecondRealTime() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_REALTIME, &tp)) {
        return ((static_cast<double>(tp.tv_nsec) / 1e9) + tp.tv_sec);
    } else {
        return 0.0;
    }
}
double FasterNowSecondRealTime() noexcept
{
    timespec tp;
    if (!::clock_gettime(CLOCK_REALTIME_COARSE, &tp)) {
        return ((static_cast<double>(tp.tv_nsec) / 1e9) + tp.tv_sec);
    } else {
        return 0.0;
    }
}
bool IsTimeout(double const deadline) noexcept
{
    return (deadline > std::numeric_limits<double>::epsilon())
        && (NowSecond() > deadline);
}
#if !defined _WIN32 || !_WIN32
double LocalTimezone(struct tm const& localctm) noexcept
{
    return localctm.tm_gmtoff / 3600.0;
}
#endif
bool UtcSecToLocalCtm(time_t const utcSec, struct tm& localctm) noexcept
{
    /*
     * Get local tm
     * use localtime_r for threads safe
     */
#   if !defined _WIN32 || !_WIN32
    struct tm* const chk = ::localtime_r(&utcSec, &localctm);
#   else
    struct tm* const chk = ::localtime(&utcSec);
    if (chk) {
        ::memcpy(&localctm, chk, sizeof(struct tm));
    }
#   endif
    if (!chk) {
        ::memset(&localctm, 0, sizeof(struct tm));
    }
    if (0 == localctm.tm_mday) {
        //  .tm_mday is 0 => 1
        localctm.tm_mday = 1;
    }
    return bool(chk);
}
std::string MakeTzLocalNanoTimestamp() noexcept
{
    timespec tp;
    if (::clock_gettime(CLOCK_REALTIME_COARSE, &tp)) {
        return "0000 00-00-00 00:00:00.000000000";
    }
    struct tm localctm;
    UtcSecToLocalCtm(tp.tv_sec, localctm);
    int const tz = int(int64_t(LocalTimezone(localctm)));
    char buf[40];
    ::snprintf(buf, sizeof(buf),
        "%02d %04d-%02d-%02d %02d:%02d:%02d.%09ld",
        tz & 0xff,
        localctm.tm_year + 1900,
        localctm.tm_mon + 1,
        localctm.tm_mday,
        localctm.tm_hour,
        localctm.tm_min,
        localctm.tm_sec,
#       if !defined __APPLE__
        tp.tv_nsec
#       else
        long(tp.tv_nsec)
#       endif
        );
    return buf;
}
std::string MakeShortTzLocalNanoTimestamp() noexcept
{
    timespec tp;
    if (::clock_gettime(CLOCK_REALTIME_COARSE, &tp)) {
        return "00 00-00-00 00:00:00.000000000";
    }
    struct tm localctm;
    UtcSecToLocalCtm(tp.tv_sec, localctm);
    int const tz = int(int64_t(LocalTimezone(localctm)));
    char buf[32];
    ::snprintf(buf, sizeof(buf),
        "%02d %02d-%02d-%02d %02d:%02d:%02d.%09ld",
        tz & 0xff,
        (localctm.tm_year + 1900) % 100,
        localctm.tm_mon + 1,
        localctm.tm_mday,
        localctm.tm_hour,
        localctm.tm_min,
        localctm.tm_sec,
#       if !defined __APPLE__
        tp.tv_nsec
#       else
        long(tp.tv_nsec)
#       endif
        );
    return buf;
}
}
