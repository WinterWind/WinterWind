/**
 * Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <chrono>
#include <cstring>
#include <string>
#include <sstream>

#define START_CHRONO const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
#define END_CHRONO const std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
#define CHRONO_DURATION const std::chrono::duration<double> elapsed_seconds = end_time - start_time;
#define PERIODIC_FUNCTION(F, TV, T, I) \
	T -= I; \
	if (T <= I) { \
        F; \
        T = TV; \
    }

inline std::string readable_chrono_duration
	(std::chrono::time_point<std::chrono::system_clock> start_time,
	 std::chrono::time_point<std::chrono::system_clock> end_time)
{
	CHRONO_DURATION;
	std::stringstream ss;
	if (elapsed_seconds.count() * 1000.f < 1.0f) {
		ss << elapsed_seconds.count() * 1000.f * 1000.0f << "µs";
	}
	else if (elapsed_seconds.count() < 1.0f) {
		ss << elapsed_seconds.count() * 1000.f << "ms";
	}
	else {
		ss << elapsed_seconds.count() << "s";
	}

	return ss.str();
}

static void timestamp_str_hour(const time_t t, std::string &res)
{
	std::tm * ptm = std::localtime(&t);
	char buffer[10] = {0};
	std::strftime(buffer, sizeof(buffer), "%H:%M:%S", ptm);
	res = std::string(buffer, strlen(buffer));
}

static void time_to_string(const time_t &t, std::string &res, const bool gmt = false)
{
	std::tm * ptm = (gmt ? std::gmtime(&t) : std::localtime(&t));
	char buffer[24] = {0};
	std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", ptm);
	res = std::string(buffer, strlen(buffer));
}

static void time_now_string(std::string &res)
{
	time_to_string(time(NULL), res);
}

static int get_hour_now()
{
	time_t now = time(NULL);
	std::tm * ptm = std::localtime(&now);
	return ptm->tm_hour;
}

#define CHRONO_DURATION_STR readable_chrono_duration(start_time, end_time)