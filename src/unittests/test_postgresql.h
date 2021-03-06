/*
 * Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <core/databases/postgresqlclient.h>

namespace winterwind {

namespace unittests {

class Test_PostgreSQL : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_PostgreSQL);
	CPPUNIT_TEST(pg_register_embedded_statements);
	CPPUNIT_TEST(pg_register_custom_statement);
	//CPPUNIT_TEST(pg_add_admin_views);
	CPPUNIT_TEST(pg_drop_schema);
	CPPUNIT_TEST(pg_create_schema);
	CPPUNIT_TEST(pg_create_table);
	CPPUNIT_TEST(pg_insert);
	CPPUNIT_TEST(pg_transaction_insert);
	CPPUNIT_TEST(pg_drop_table);
	CPPUNIT_TEST(pg_show_tables);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() override {}

	void tearDown() override;

protected:
	void pg_register_embedded_statements();
	void pg_register_custom_statement();
	void pg_add_admin_views();
	void pg_drop_schema();
	void pg_create_schema();
	void pg_create_table();
	void pg_drop_table();
	void pg_insert();
	void pg_transaction_insert();
	void pg_show_tables();
};
}
}
