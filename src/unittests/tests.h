/**
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

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>


#include <gitlabapiclient.h>
#include <httpserver.h>
#include <utils/stringutils.h>
#include <elasticsearchclient.h>
#include <console.h>
#include <openweathermapclient.h>
#include <luaengine.h>
#include <postgresqlclient.h>
#include <twitterclient.h>
#include <utils/base64.h>

#include "unittests_config.h"

#define CPPUNIT_TESTSUITE_CREATE(s) CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite(std::string(s));
#define CPPUNIT_ADDTEST(c, s, f) suiteOfTests->addTest(new CppUnit::TestCaller<c>(s, &c::f));

static std::string TWITTER_CONSUMER_KEY = "";
static std::string TWITTER_CONSUMER_SECRET = "";
static std::string TWITTER_ACCESS_TOKEN = "";
static std::string TWITTER_ACCESS_TOKEN_SECRET = "";

class WinterWindTests: public CppUnit::TestFixture
{
public:
	WinterWindTests() {}
	virtual ~WinterWindTests()
	{
		delete m_twitter_client;
		delete m_http_server;
	}

	static CppUnit::Test *suite()
	{
		CPPUNIT_TESTSUITE_CREATE("WinterWind")

		CPPUNIT_ADDTEST(WinterWindTests, "Weather - Test1", weather_to_json);

		CPPUNIT_ADDTEST(WinterWindTests, "Twitter - Test1 - Authenticate", twitter_authenticate);
		CPPUNIT_ADDTEST(WinterWindTests, "Twitter - Test2 - User Timeline", twitter_user_timeline);
		CPPUNIT_ADDTEST(WinterWindTests, "Twitter - Test3 - Home Timeline", twitter_home_timeline);

		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test1 - Handle GET", httpserver_handle_get);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test2 - Test headers", httpserver_header);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test3 - Test get params", httpserver_getparam);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test4 - Handle POST (form encoded)", httpserver_handle_post);
		CPPUNIT_ADDTEST(WinterWindTests, "HTTPServer - Test5 - Handle POST (json)", httpserver_handle_post_json);

		CPPUNIT_ADDTEST(WinterWindTests, "LuaEngine - Test1 - Load winterwind engine", lua_winterwind_engine)

		return suiteOfTests;
	}

	void setUp()
	{
		m_twitter_client = new TwitterClient(TWITTER_CONSUMER_KEY, TWITTER_CONSUMER_SECRET,
			TWITTER_ACCESS_TOKEN, TWITTER_ACCESS_TOKEN_SECRET);
		m_http_server = new HTTPServer(58080);
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest.html",
			&WinterWindTests::httpserver_testhandler, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest2.html",
			&WinterWindTests::httpserver_testhandler2, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, GET, "/unittest3.html",
			&WinterWindTests::httpserver_testhandler3, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, POST, "/unittest4.html",
			&WinterWindTests::httpserver_testhandler4, this)
		BIND_HTTPSERVER_HANDLER(m_http_server, POST, "/unittest5.html",
			&WinterWindTests::httpserver_testhandler5, this)
	}

	void tearDown() {}

protected:
	void lua_winterwind_engine()
	{
		LuaEngine L;
		LuaReturnCode  rc = L.init_winterwind_bindings();
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		rc = L.load_script(UNITTESTS_LUA_FILE);
		CPPUNIT_ASSERT(rc == LUA_RC_OK);
		CPPUNIT_ASSERT(L.run_unittests());
	}

	void twitter_authenticate()
	{
		CPPUNIT_ASSERT(m_twitter_client->authenticate() == TwitterClient::TWITTER_OK);
	}

	void twitter_user_timeline()
	{
		twitter_authenticate();
		Json::Value res;
		CPPUNIT_ASSERT(m_twitter_client->get_user_timeline(res, 10, 0, true) == TwitterClient::TWITTER_OK);
	}

	void twitter_home_timeline()
	{
		twitter_authenticate();
		Json::Value res;
		CPPUNIT_ASSERT(m_twitter_client->get_home_timeline(res, 10) == TwitterClient::TWITTER_OK);
		CPPUNIT_ASSERT(res.isObject() || res.isArray());
	}

	void weather_to_json()
	{
		Weather w;
		w.sunset = 150;
		w.sunrise = 188;
		w.humidity = 4;
		w.temperature = 25.0f;
		w.city = "test_city";
		Json::Value res;
		w >> res;
		CPPUNIT_ASSERT(res["sunset"].asUInt() == 150);
		CPPUNIT_ASSERT(res["sunrise"].asUInt() == 188);
		CPPUNIT_ASSERT(res["humidity"].asInt() == 4);
		CPPUNIT_ASSERT(res["temperature"].asFloat() == 25.0f);
		CPPUNIT_ASSERT(res["city"].asString() == "test_city");
	}

	HTTPResponse *httpserver_testhandler(const HTTPQueryPtr q)
	{
		return new HTTPResponse(HTTPSERVER_TEST01_STR);
	}

	HTTPResponse *httpserver_testhandler2(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->headers.find("UnitTest-Header");
		if (it != q->headers.end() && it->second == "1") {
			res = "yes";
		}
		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler3(const HTTPQueryPtr q)
	{
		std::string res = "no";

		const auto it = q->get_params.find("UnitTestParam");
		if (it != q->get_params.end() && it->second == "thisistestparam") {
			res = "yes";
		}

		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler4(const HTTPQueryPtr q)
	{
		std::string res = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_FORM);

		HTTPFormQuery *fq = dynamic_cast<HTTPFormQuery *>(q.get());
		CPPUNIT_ASSERT(fq);

		const auto it = fq->post_data.find("post_param");
		if (it != fq->post_data.end() && it->second == "ilikedogs") {
			res = "yes";
		}

		return new HTTPResponse(res);
	}

	HTTPResponse *httpserver_testhandler5(const HTTPQueryPtr q)
	{
		Json::Value json_res;
		json_res["status"] = "no";
		CPPUNIT_ASSERT(q->get_type() == HTTPQUERY_TYPE_JSON);

		HTTPJsonQuery *jq = dynamic_cast<HTTPJsonQuery *>(q.get());
		CPPUNIT_ASSERT(jq);

		if (jq->json_query.isMember("json_param")
			&& jq->json_query["json_param"].asString() == "catsarebeautiful") {
			json_res["status"] = "yes";
		}

		return new JSONHTTPResponse(json_res);
	}

	void httpserver_handle_get()
	{
		HTTPClient cli;
		std::string res;
		cli._get("http://localhost:58080/unittest.html", res);
		CPPUNIT_ASSERT(res == HTTPSERVER_TEST01_STR);
	}

	void httpserver_header()
	{
		HTTPClient cli;
		std::string res;
		cli.add_http_header("UnitTest-Header", "1");
		cli._get("http://localhost:58080/unittest2.html", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_getparam()
	{
		HTTPClient cli;
		std::string res;
		cli._get("http://localhost:58080/unittest3.html?UnitTestParam=thisistestparam", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_handle_post()
	{
		HTTPClient cli;
		std::string res;
		cli.add_http_header("Content-Type", "application/x-www-form-urlencoded");
		cli.add_form_param("post_param", "ilikedogs");
		cli._post("http://localhost:58080/unittest4.html", res);
		CPPUNIT_ASSERT(res == "yes");
	}

	void httpserver_handle_post_json()
	{
		HTTPClient cli;
		Json::Value query;
		query["json_param"] = "catsarebeautiful";
		Json::Value res;
		cli._post_json("http://localhost:58080/unittest5.html", query, res);
		CPPUNIT_ASSERT(res.isMember("status") && res["status"] == "yes");
	}
private:
	TwitterClient *m_twitter_client = nullptr;
	HTTPServer *m_http_server = nullptr;
	std::string HTTPSERVER_TEST01_STR = "<h1>unittest_result</h1>";
};
