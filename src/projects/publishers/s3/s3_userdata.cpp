#include "s3_userdata.h"

#include <base/info/stream.h>
#include <base/publisher/stream.h>

#include "s3_private.h"

S3UserdataSets::S3UserdataSets()
{
}

S3UserdataSets::~S3UserdataSets()
{
	_sets.clear();
}

bool S3UserdataSets::Set(std::shared_ptr<info::Push> userdata)
{
	std::lock_guard<std::shared_mutex> lock(_mutex);

	_sets[userdata->GetId()] = userdata;

	return true;
}

std::shared_ptr<info::Push> S3UserdataSets::GetByKey(ov::String key)
{
	std::lock_guard<std::shared_mutex> lock(_mutex);

	auto iter = _sets.find(key);
	if (iter == _sets.end())
	{
		return nullptr;
	}

	return iter->second;
}

std::shared_ptr<info::Push> S3UserdataSets::GetBySessionId(session_id_t session_id)
{
	std::lock_guard<std::shared_mutex> lock(_mutex);
	for (auto &item : _sets)
	{
		auto userdata = item.second;

		if (userdata->GetSessionId() == session_id)
			return userdata;
	}

	return nullptr;
}

std::shared_ptr<info::Push> S3UserdataSets::GetAt(uint32_t index)
{
	std::lock_guard<std::shared_mutex> lock(_mutex);

	auto iter = _sets.begin();

	std::advance(iter, index);

	if (iter == _sets.end())
		return nullptr;

	return iter->second;
}

void S3UserdataSets::DeleteByKey(ov::String key)
{
	std::lock_guard<std::shared_mutex> lock(_mutex);

	_sets.erase(key);
}

uint32_t S3UserdataSets::GetCount()
{
	std::lock_guard<std::shared_mutex> lock(_mutex);

	return _sets.size();
}
