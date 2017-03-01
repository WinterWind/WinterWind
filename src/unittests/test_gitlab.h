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
#include <cppunit/extensions/HelperMacros.h>

#include <gitlabapiclient.h>

static std::string GITLAB_TOKEN = "";
static std::string RUN_TIMESTAMP = std::to_string(time(NULL));

class WinterWindTest_Gitlab: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(WinterWindTest_Gitlab);
	CPPUNIT_TEST(create_default_groups);
	CPPUNIT_TEST(create_group);
	CPPUNIT_TEST(get_group);
	CPPUNIT_TEST(get_namespaces);

	CPPUNIT_TEST(create_default_projects);
	CPPUNIT_TEST(create_project);
	CPPUNIT_TEST(get_projects);
	CPPUNIT_TEST(get_project);
	CPPUNIT_TEST(get_project_ns);

	CPPUNIT_TEST(create_label);
	CPPUNIT_TEST(get_label);
	CPPUNIT_TEST(remove_label);

	CPPUNIT_TEST(remove_project);
	CPPUNIT_TEST(remove_projects);

	CPPUNIT_TEST(remove_group);
	CPPUNIT_TEST(remove_groups);

	CPPUNIT_TEST_SUITE_END();
public:
	void setUp()
	{
		m_gitlab_client = new GitlabAPIClient("https://gitlab.com", GITLAB_TOKEN);
	}

	void tearDown()
	{
		delete m_gitlab_client;
	}

protected:
	void create_default_groups()
	{
		Json::Value res;
		GitlabGroup g("ww_testgroup_default_" + RUN_TIMESTAMP, "ww_testgroup_default_" + RUN_TIMESTAMP);
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res));

		GitlabGroup g2("ww_testgroup2_default_" + RUN_TIMESTAMP, "ww_testgroup2_default_" + RUN_TIMESTAMP);
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g2, res));
	}

	void create_group()
	{
		Json::Value res;
		GitlabGroup g(TEST_GROUP, TEST_GROUP);
		g.description = "test";
		g.visibility = GITLAB_GROUP_PUBLIC;
		CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res));
	}

	void get_group()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_group(
				std::string("ww_testgroup_") + RUN_TIMESTAMP, result) == GITLAB_RC_OK);
	}

	void remove_group()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_group(TEST_GROUP) == GITLAB_RC_OK);
	}

	void remove_groups()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_groups({
				"ww_testgroup_default_" + RUN_TIMESTAMP,
				"ww_testgroup2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK);
	}

	void get_namespaces()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_namespaces("", result) == GITLAB_RC_OK);
	}

	void get_namespace()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_namespace(
				std::string("ww_testgroup_") + RUN_TIMESTAMP, result) == GITLAB_RC_OK);

		m_testing_namespace_id = result["id"].asUInt();
	}

	void create_default_projects()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->create_project(GitlabProject("ww_testproj1_default_" + RUN_TIMESTAMP), res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(m_gitlab_client->create_project(GitlabProject("ww_testproj2_default_" + RUN_TIMESTAMP), res) == GITLAB_RC_OK);
	}

	void create_project()
	{
		// Required to get the namespace on which create the project
		get_namespace();

		Json::Value res;
		GitlabProject proj("ww_testproj_" + RUN_TIMESTAMP);
		proj.builds_enabled = false;
		proj.visibility_level = GITLAB_PROJECT_INTERNAL;
		proj.merge_requests_enabled = false;
		proj.issues_enabled = true;
		proj.lfs_enabled = true;
		proj.description = "Amazing description";
		proj.only_allow_merge_if_all_discussions_are_resolved = true;
		proj.namespace_id = m_testing_namespace_id;
		CPPUNIT_ASSERT(m_gitlab_client->create_project(proj, res) == GITLAB_RC_OK);
	}

	void get_projects()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_projects("ww_testproj", res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res[0].isMember("http_url_to_repo"));
	}

	void get_project()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project("ww_testproj1_default_" + RUN_TIMESTAMP, res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("name_with_namespace"));
	}

	void get_project_ns()
	{
		Json::Value res;
		CPPUNIT_ASSERT(m_gitlab_client->get_project_ns(
			"ww_testproj_" + RUN_TIMESTAMP, TEST_GROUP, res) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(res.isMember("avatar_url"));
	}

	void remove_project()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_project(
				std::string("ww_testproj_") + RUN_TIMESTAMP) == GITLAB_RC_OK);
	}

	void remove_projects()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_projects({
				"ww_testproj1_default_" + RUN_TIMESTAMP,
				"ww_testproj2_default_" + RUN_TIMESTAMP}) == GITLAB_RC_OK);
	}

	void create_label()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->create_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL, TEST_COLOR, result)
				== GITLAB_RC_OK);
	}

	void get_label()
	{
		Json::Value result;
		CPPUNIT_ASSERT(m_gitlab_client->get_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL, result) == GITLAB_RC_OK);
		CPPUNIT_ASSERT(result.isMember("color") && result["color"].asString() == TEST_COLOR);
	}

	void remove_label()
	{
		CPPUNIT_ASSERT(m_gitlab_client->delete_label(TEST_GROUP,
				"ww_testproj_" + RUN_TIMESTAMP, TEST_LABEL) == GITLAB_RC_OK);
	}
private:
	GitlabAPIClient *m_gitlab_client = nullptr;
	uint32_t m_testing_namespace_id = 0;
	std::string TEST_COLOR = "#005577";
	std::string TEST_GROUP = "ww_testgroup_" + RUN_TIMESTAMP;
	std::string TEST_LABEL = "test_label_1";
};
