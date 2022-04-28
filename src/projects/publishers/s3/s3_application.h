#pragma once

#include <base/common_types.h>
#include <base/info/session.h>
#include <base/publisher/application.h>

#include "s3_stream.h"
#include "s3_userdata.h"

class S3Application : public pub::PushApplication
{
public:
	static std::shared_ptr<S3Application> Create(const std::shared_ptr<pub::Publisher> &publisher, const info::Application &application_info);
	S3Application(const std::shared_ptr<pub::Publisher> &publisher, const info::Application &application_info);
	~S3Application() final;

private:
	bool Start() override;
	bool Stop() override;

	// Application Implementation
	std::shared_ptr<pub::Stream> CreateStream(const std::shared_ptr<info::Stream> &info, uint32_t worker_count) override;
	bool DeleteStream(const std::shared_ptr<info::Stream> &info) override;

public:
	void SessionUpdateByUser();
	void SessionUpdateByStream(std::shared_ptr<S3Stream> stream, bool stopped);
	void SessionUpdate(std::shared_ptr<S3Stream> stream, std::shared_ptr<info::Push> userdata);
	void SessionStart(std::shared_ptr<S3Session> session);
	void SessionStop(std::shared_ptr<S3Session> session);

	virtual std::shared_ptr<ov::Error> PushStart(const std::shared_ptr<info::Push> &push) override;
	virtual std::shared_ptr<ov::Error> PushStop(const std::shared_ptr<info::Push> &push) override;
	virtual std::shared_ptr<ov::Error> GetPushes(const std::shared_ptr<info::Push> push, std::vector<std::shared_ptr<info::Push>> &results) override;

private:
	S3UserdataSets _userdata_sets;
};
