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

#include <unordered_map>
#include <core/utils/base64.h>
#include <cassert>
#include <core/httpclient.h>
#include <core/utils/hmac.h>
#include <core/utils/stringutils.h>
#include <iostream>
#include "utils/jsonwebtokens.h"

namespace winterwind {
namespace web {

static const std::string jwt_alg_str[JsonWebToken::Algorithm::JWT_ALG_MAX] {
	"HS256",
	"HS384",
	"HS512",
};

JsonWebToken::JsonWebToken(Algorithm alg, const Json::Value &payload,
		const std::string &secret):
	m_algorithm(alg),
	m_payload(payload),
	m_secret(secret)
{
}

JsonWebToken::GenStatus JsonWebToken::get(std::string &result) const
{
	assert(m_algorithm < Algorithm::JWT_ALG_MAX);

	// Forbid JWT reserved claim in payload
	for (const auto &m: {"iss", "exp", "iat", "sub"}) {
		if (m_payload.isMember(m)) {
			return GenStatus::GENSTATUS_JWT_CLAIM_IN_PAYLOAD;
		}
	}

	Json::FastWriter json_writer;
	std::string tmp_result;

	{
		Json::Value header;
		header["alg"] = jwt_alg_str[m_algorithm];
		header["typ"] = "JWT";

		tmp_result = base64_urlencode(json_writer.write(header)) + ".";
	}

	// Copy payload to add reserved claims
	Json::Value final_payload = m_payload;

	// Add reserved claims
	if (m_subject_set) {
		final_payload["sub"] = m_subject;
	}

	if (m_issuer_set) {
		final_payload["iss"] = m_issuer;
	}

	if (m_expirationtime_set) {
		final_payload["exp"] = m_expiration_time;
	}

	if (m_issued_at_set) {
		final_payload["iat"] = m_issued_at;
	}

	// concat payload with header
	tmp_result += base64_urlencode(json_writer.write(final_payload));

	// Sign from current concat header . payload
	std::string signature;
	sign(tmp_result, signature);

	tmp_result += "." + base64_encode(signature);

	// Strip '=' char from the whole string
	str_remove_substr(tmp_result, "=");

	result = std::move(tmp_result);
	return GenStatus::GENSTATUS_OK;
}

void JsonWebToken::sign(const std::string &payload, std::string &signature) const
{
	switch (m_algorithm) {
		case ALG_HS256:
			signature = std::move(hmac_sha256(m_secret, payload));
			break;
		case ALG_HS384:
			signature = std::move(hmac_sha384(m_secret, payload));
			break;
		case ALG_HS512:
			signature = std::move(hmac_sha512(m_secret, payload));
			break;
		default: assert(false);
	}
}

JsonWebToken & JsonWebToken::issuedAt(std::time_t when)
{
	m_issued_at = when;
	m_issuer_set = true;
	return *this;
}

JsonWebToken & JsonWebToken::expirationTime(std::time_t when)
{
	m_expiration_time = when;
	m_expirationtime_set = true;
	return *this;
}

JsonWebToken& JsonWebToken::subject(const std::string &subject)
{
	m_subject = subject;
	m_subject_set = true;
	return *this;
}

JsonWebToken& JsonWebToken::issuer(const std::string &issuer)
{
	m_issuer = issuer;
	m_issuer_set = true;
	return *this;
}

/*
 * JWTDecoder
 */

JWTDecoder& JWTDecoder::add_issuer_validator(JWTValidationFctStr &f)
{
	m_issuer_validators.push_back(f);
	return *this;
}

JWTDecoder& JWTDecoder::add_subject_validator(JWTValidationFctStr &f)
{
	m_subject_validators.push_back(f);
	return *this;
}

JWTDecoder& JWTDecoder::add_expiration_time_validator(JWTValidationFctTime &f)
{
	m_expiration_time_validators.push_back(f);
	return *this;
}

JWTDecoder& JWTDecoder::add_issued_at_validator(JWTValidationFctTime &f)
{
	m_issued_at_validators.push_back(f);
	return *this;
}

JWTDecoder::JWTStatus JWTDecoder::read_and_verify(std::string raw_token,
	JsonWebToken &jwt)
{
	// Strip '=' char
	str_remove_substr(raw_token, "=");

	std::vector<std::string> res = {};
	str_split(raw_token, '.', res);
	if (res.size() != 3) {
		return STATUS_INVALID_STRING;
	}

	Json::Reader reader;
	if (!reader.parse(base64_urldecode(res[0]), jwt.m_header)) {
		return STATUS_JSON_PARSE_ERROR;
	}

	if (!jwt.m_header.isMember("alg") || !jwt.m_header["alg"].isString() ||
		!jwt.m_header.isMember("typ") || !jwt.m_header["typ"].isString() ||
		jwt.m_header["typ"].asString() != "JWT") {
		return STATUS_INVALID_HEADER;
	}

	{
		bool alg_found = false;
		const std::string &alg = jwt.m_header["alg"].asString();
		for (uint8_t i = 0; i < JsonWebToken::Algorithm::JWT_ALG_MAX; i++) {
			if (jwt_alg_str[i] == alg) {
				jwt.m_algorithm = (JsonWebToken::Algorithm) i;
				alg_found = true;
				break;
			}
		}

		if (!alg_found) {
			return STATUS_INVALID_ALGORITHM;
		}
	}

	Json::Value payload;
	if (!reader.parse(base64_urldecode(res[1]), jwt.m_payload)) {
		return STATUS_JSON_PARSE_ERROR;
	}

	std::string signature = base64_urlencode(hmac_sha256(jwt.m_secret, res[0] + "." + res[1]));
	// Strip '=' char
	str_remove_substr(signature, "=");

	if (signature != res[2]) {
		return STATUS_INVALID_SIGNATURE;
	}

	// If issuer if set, validate type and check all callbacks
	if (payload.isMember("iss")) {
		if (!payload["iss"].isString()) {
			return STATUS_INVALID_CLAIM;
		}

		for (const auto &cb: m_issuer_validators) {
			if (!cb(payload["iss"].asString())) {
				return STATUS_ISSUER_ERROR;
			}
		}
	}

	// If subject if set, validate type and check all callbacks
	if (payload.isMember("sub")) {
		if (!payload["sub"].isString()) {
			return STATUS_INVALID_CLAIM;
		}

		for (const auto &cb: m_subject_validators) {
			if (!cb(payload["sub"].asString())) {
				return STATUS_SUBJECT_ERROR;
			}
		}
	}

	// If expire if set, validate type and check all callbacks
	if (payload.isMember("exp")) {
		if (!payload["exp"].isUInt64()) {
			return STATUS_INVALID_CLAIM;
		}

		for (const auto &cb: m_expiration_time_validators) {
			if (!cb(payload["exp"].asUInt64())) {
				return STATUS_TOKEN_EXPIRED;
			}
		}
	}

	// If issued_at if set, validate type and check all callbacks
	if (payload.isMember("iat")) {
		if (!payload["iat"].isUInt64()) {
			return STATUS_INVALID_CLAIM;
		}

		for (const auto &cb: m_issued_at_validators) {
			if (!cb(payload["iat"].asUInt64())) {
				return STATUS_ISSUED_AT_ERROR;
			}
		}
	}

	return STATUS_OK;
}

}
}
