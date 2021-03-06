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

#include "test_elasticsearch.h"

#include <extras/elasticsearchclient.h>

using namespace winterwind::extras;

namespace winterwind {
namespace unittests {

void Test_Elasticsearch::setUp()
{
	if (!getenv("ES_HOST")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing Elasticsearch host, defaulting to "
			<< m_es_host << std::endl;
		return;
	}

	m_es_host = std::string(getenv("ES_HOST"));
}

void Test_Elasticsearch::tearDown()
{
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	{
		elasticsearch::Index index("index_unittests", &client);
		index.remove();
	}
	{
		elasticsearch::Index index("analyze_unittests", &client);
		index.remove();
	}
	{
		elasticsearch::Index index("library", &client);
		index.remove();
	}
}

void Test_Elasticsearch::es_bulk_to_json()
{
	ElasticsearchBulkAction action(ESBULK_AT_INDEX);
	action.index = "library";
	action.type = "book";
	action.doc_id = "7";
	action.doc["title"] = "A great history";

	Json::FastWriter writer;
	std::string res;
	action.toJson(writer, res);

	CPPUNIT_ASSERT(res ==
			"{\"index\":{\"_id\":\"7\",\"_index\":\"library\",\"_type\":\"book\"}}\n"
			"{\"title\":\"A great history\"}\n");
}

void Test_Elasticsearch::es_bulk_update_to_json()
{
	ElasticsearchBulkAction action(ESBULK_AT_UPDATE);
	action.index = "car";
	action.type = "truck";
	action.doc_id = "666";
	action.doc["engine"] = "Toyota";

	Json::FastWriter writer;
	std::string res;
	action.toJson(writer, res);

	CPPUNIT_ASSERT(res == "{\"update\":{\"_id\":\"666\",\"_index\":\"car\",\"_type\":\"truck\"}}\n"
		"{\"doc\":{\"engine\":\"Toyota\"}}\n");
}

void Test_Elasticsearch::es_bulk_delete_to_json()
{
	ElasticsearchBulkAction action(ESBULK_AT_DELETE);
	action.index = "food";
	action.type = "meat";
	action.doc_id = "5877";

	Json::FastWriter writer;
	std::string res;
	action.toJson(writer, res);

	CPPUNIT_ASSERT(res == "{\"delete\":{\"_id\":\"5877\",\"_index\":\"food\",\"_type\":\"meat\"}}\n");
}

void Test_Elasticsearch::es_bulk_play_index()
{
	ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_INDEX));
	action->index = "library";
	action->type = "book";
	action->doc_id = "7";
	action->doc["title"] = "A great history";

	std::string bulk_res;
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	client.add_bulkaction_to_queue(action);
	client.process_bulkaction_queue(bulk_res);

	Json::Reader reader;
	Json::Value es_res;
	CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
	CPPUNIT_ASSERT(es_res.isMember("errors"));
	CPPUNIT_ASSERT(es_res["errors"].isBool());
	CPPUNIT_ASSERT(!es_res["errors"].asBool());
}

void Test_Elasticsearch::es_bulk_play_update()
{
	ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_INDEX));
	action->index = "library";
	action->type = "book";
	action->doc_id = "7";
	action->doc["title"] = "A great history (version 2)";

	ElasticsearchBulkActionPtr action2(new ElasticsearchBulkAction(ESBULK_AT_UPDATE));
	action2->index = "library";
	action2->type = "book";
	action2->doc_id = "7";
	action2->doc["title"] = "A great history (version 3)";

	std::string bulk_res;
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	client.add_bulkaction_to_queue(action);
	client.add_bulkaction_to_queue(action2);
	client.process_bulkaction_queue(bulk_res);

	Json::Reader reader;
	Json::Value es_res;
	CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
	CPPUNIT_ASSERT(es_res.isMember("errors"));
	CPPUNIT_ASSERT(es_res["errors"].isBool());
	CPPUNIT_ASSERT(!es_res["errors"].asBool());
}

void Test_Elasticsearch::es_bulk_play_delete()
{
	ElasticsearchBulkActionPtr action(new ElasticsearchBulkAction(ESBULK_AT_DELETE));
	action->index = "library";
	action->type = "book";
	action->doc_id = "7";

	std::string bulk_res;
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	client.add_bulkaction_to_queue(action);
	client.process_bulkaction_queue(bulk_res);

	Json::Reader reader;
	Json::Value es_res;
	CPPUNIT_ASSERT(reader.parse(bulk_res, es_res));
	CPPUNIT_ASSERT(es_res.isMember("errors"));
	CPPUNIT_ASSERT(es_res["errors"].isBool());
	CPPUNIT_ASSERT(!es_res["errors"].asBool());
}

void Test_Elasticsearch::es_create_index()
{
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	elasticsearch::Index ut_index("index_unittests", &client);

	// Verify if not exists
	CPPUNIT_ASSERT(!ut_index.exists());

	// Create
	CPPUNIT_ASSERT(ut_index.create());

	// Ensure it's created
	CPPUNIT_ASSERT(ut_index.exists());

	// Remove
	CPPUNIT_ASSERT(ut_index.remove());

	// Ensure it's removed
	CPPUNIT_ASSERT(!ut_index.exists());

	// Set shard count
	CPPUNIT_ASSERT(ut_index.set_shard_count(10));

	// Create with new shard count
	CPPUNIT_ASSERT(ut_index.create());
}

void Test_Elasticsearch::es_analyze()
{
	ElasticsearchClient client("http://" + m_es_host + ":9200");
	elasticsearch::Index ut_index("analyze_unittests", &client);
	std::vector<std::string> filters = {"lowercase"};
	elasticsearch::AnalyzerPtr autocomplete_analyzer =
		std::make_shared<elasticsearch::Analyzer>("autocomplete", "standard", filters);
	ut_index.add_analyzer(autocomplete_analyzer);

	CPPUNIT_ASSERT(ut_index.create());

	Json::Value res;
	bool status = client.analyze("analyze_unittests", "autocomplete", "unittests are nice!", res);

	CPPUNIT_ASSERT_MESSAGE("Analyze failed, bad request", status);
	CPPUNIT_ASSERT_MESSAGE("Analyze failed, invalid response: "
		+ res.toStyledString(), res.isMember("tokens") && res["tokens"].isArray());
}
}
}
