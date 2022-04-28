#pragma once

#include <base/info/media_track.h>
#include <base/publisher/session.h>
#include <modules/s3/s3_writer.h>

#include "base/info/push.h"

class S3Session : public pub::Session
{
public:
	static std::shared_ptr<S3Session> Create(const std::shared_ptr<pub::Application> &application,
											 const std::shared_ptr<pub::Stream> &stream,
											 uint32_t ovt_session_id);

	S3Session(const info::Session &session_info,
			  const std::shared_ptr<pub::Application> &application,
			  const std::shared_ptr<pub::Stream> &stream);
	~S3Session() override;

	bool Start() override;
	bool Stop() override;

	bool SendOutgoingData(const std::any &packet) override;
	void OnPacketReceived(const std::shared_ptr<info::Session> &session_info,
						  const std::shared_ptr<const ov::Data> &data) override;

	void SetPush(std::shared_ptr<info::Push> &record);
	std::shared_ptr<info::Push> &GetPush();

private:
	std::shared_ptr<info::Push> _push;

	std::shared_mutex _mutex;

	std::shared_ptr<s3Writer> _writer;
};
