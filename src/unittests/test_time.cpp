/*
 * Copyright (c) 2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include "test_time.h"

#include <core/utils/time.h>

namespace winterwind {
namespace unittests {

void Test_Time::setUp()
{
	setenv("TZ", "/usr/share/zoneinfo/Europe/Paris", 1);
}

void Test_Time::time_str_to_timestamp()
{
	std::time_t t;
	bool res = str_to_timestamp("2017-01-24 05:00:47", t);
	CPPUNIT_ASSERT(res);
	CPPUNIT_ASSERT(t == 1485230447);

	res = str_to_timestamp("2017-01-24T05:00:49Z", t);
	CPPUNIT_ASSERT(res);
	CPPUNIT_ASSERT(t == 1485230449);

	res = str_to_timestamp("1-1-1 00:00:00", t);
	CPPUNIT_ASSERT(!res);

	res = str_to_timestamp("017-01-24 36:00:00", t);
	CPPUNIT_ASSERT(!res);

	res = str_to_timestamp("017-01-24 05:68:00", t);
	CPPUNIT_ASSERT(!res);

	res = str_to_timestamp("017-01-24 05:28:99", t);
	CPPUNIT_ASSERT(!res);
}

void Test_Time::time_get_current_hour()
{
	int hour = get_current_hour();
	CPPUNIT_ASSERT(hour >= 0 && hour < 24);
}

}
}
