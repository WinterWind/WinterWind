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

#include <core/httpclient.h>
#include <core/utils/classhelpers.h>
#include <core/utils/exception.h>
#include <chrono>
#include <memory>
#include <queue>

namespace winterwind
{
namespace extras
{
class ElasticsearchException : public BaseException
{
public:
	ElasticsearchException(const std::string &what) : BaseException(what)
	{}

	~ElasticsearchException() throw()
	{}
};

struct ElasticsearchNode
{
public:
	ElasticsearchNode(const std::string &id) : tech_id(id)
	{}

	std::string http_addr = "";
	std::string tech_id;
	std::string version = "";
	bool is_master = false;

	std::string to_string() const
	{
		return "id: " + tech_id + " http_addr: " + http_addr + " version: " + version +
			" master: " +
			std::to_string(is_master);
	};
};

enum ElasticsearchBulkActionType
{
	ESBULK_AT_CREATE,
	ESBULK_AT_DELETE,
	ESBULK_AT_INDEX,
	ESBULK_AT_UPDATE,
	ESBULK_AT_MAX,
};

struct ElasticsearchBulkAction
{
public:
	ElasticsearchBulkAction(const ElasticsearchBulkActionType a) : action(a)
	{}

	std::string index = "";
	std::string type = "";
	std::string doc_id = "";
	Json::Value doc;

	void toJson(Json::FastWriter &writer, std::string &res);

private:
	ElasticsearchBulkActionType action;
};

typedef std::shared_ptr<ElasticsearchBulkAction> ElasticsearchBulkActionPtr;

class ElasticsearchClient : public http::HTTPClient
{
public:
	ElasticsearchClient(const std::string &url);

	~ElasticsearchClient();

	void discover_cluster();

	void create_doc(const std::string &index, const std::string &type,
					const Json::Value &doc);

	void insert_doc(const std::string &index, const std::string &type,
		const std::string &doc_id,
		const Json::Value &doc);

	void delete_doc(const std::string &index, const std::string &type,
		const std::string &doc_id);

	bool analyze(const std::string &index, const std::string &analyzer,
		const std::string &str, Json::Value &res);

	void add_bulkaction_to_queue(const ElasticsearchBulkActionPtr &action)
	{ m_bulk_queue.push(action); }

	void process_bulkaction_queue(std::string &res, uint32_t actions_limit = 0);

private:
	// This function permits to obtain a fresh node on which perform a query
	const ElasticsearchNode &get_fresh_node();

	std::string m_init_url = "";
	std::chrono::time_point<std::chrono::system_clock> m_last_discovery_time;

	std::vector<ElasticsearchNode> m_nodes;
	std::queue<ElasticsearchBulkActionPtr> m_bulk_queue;

	CL_HELPER_VAR_GET(std::string, cluster_name, "");
};

}
}
