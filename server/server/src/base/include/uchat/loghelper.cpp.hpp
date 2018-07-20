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
/**
 * @file loghelper.cpp.hpp
 * @note Use in CPP file !!
 *
 * - 1 include uchat/logger.hpp
 * - 2 include this file
 * - 3 define a const char* constexpr or string named kN
 */
#pragma once
#ifndef UCHAT_LOGGER_HPP
#   error("Please include uchat/logger.hpp before this file")
#endif
#include "slog/slog_api.h"
extern CSLog g_imlog;
/// @def Assert assert, throw a exception when fail
#define Assert(cond, msg) if (!(cond)) { \
    std::stringstream ss; \
    ss << msg; \
    std::string e = std::string(#cond " fail: "); \
    e += ss.str(); \
    e += " (" __FILE__ "+"; \
    e += std::to_string(__LINE__) + ")"; \
    uchat::Logger::getLogger().e(kN, e); \
    g_imlog.Error("%s", e.c_str()); \
    throw std::runtime_error(e); \
}
/// @def Throw
#define Throw(msg) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    uchat::Logger::getLogger().e(kN, ss.str()); \
    g_imlog.Error("%s", ss.str().c_str()); \
    throw std::runtime_error(ss.str()); \
}
/// @def Fatal output fatal log
#define Fatal(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Fata)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Fatal("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().f(kN, ss.str()); \
}
/// @def Error0 output error log
#define Error0(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Erro)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Error("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().e(kN, ss.str()); \
}
/// @def Warning output warning log
#define Warning(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Warn)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Warn("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().w(kN, ss.str()); \
}
/// @def Note output note log
#define Note(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Note)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Info("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().n(kN, ss.str()); \
}
/// @def Info0 output info log
#define Info0(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Info)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Info("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().i(kN, ss.str()); \
}
/// @def Trace0 output trace log
#define Trace0() if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Trac)) { \
    uchat::Logger::getLogger().append(\
        kN, __FILE__, __LINE__, __func__, uchat::LogLevel::Trac); \
    g_imlog.Trace("%s+%d: %s()", __FILE__, __LINE__, __func__); \
}
/// @def Debug output debug log
#define Debug(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Debu)) { \
    std::stringstream ss; \
    ss << msg << " (" << __FILE__ << "+" << __LINE__ << ")"; \
    g_imlog.Debug("%s", ss.str().c_str()); \
    uchat::Logger::getLogger().d(kN, ss.str()); \
}
/// @def Detail output debug log
#define Detail(msg) if (uchat::Logger::getLogger().isLogable(uchat::LogLevel::Deta)) { \
    std::stringstream ss; \
    ss << msg; \
    uchat::Logger::getLogger().append(kN, __FILE__, __LINE__, ss.str(), \
        uchat::LogLevel::Deta); \
    g_imlog.Trace("%s", ss.str().c_str()); \
}
