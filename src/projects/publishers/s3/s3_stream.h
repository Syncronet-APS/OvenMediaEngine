#pragma once

#include <base/common_types.h>
#include <base/publisher/stream.h>
#include <modules/s3/s3_writer.h>

#include "monitoring/monitoring.h"
#include "s3_session.h"

class S3Stream : public pub::Stream
{
public:
	static std::shared_ptr<S3Stream> Create(const std::shared_ptr<pub::Application> application,
											const info::Stream &info);

	explicit S3Stream(const std::shared_ptr<pub::Application> application,
					  const info::Stream &info);
	~S3Stream() final;

	void SendFrame(const std::shared_ptr<MediaPacket> &media_packet);
	void SendVideoFrame(const std::shared_ptr<MediaPacket> &media_packet) override;
	void SendAudioFrame(const std::shared_ptr<MediaPacket> &media_packet) override;

	std::shared_ptr<S3Session> CreateSession();
	bool DeleteSession(uint32_t session_id);

private:
	bool Start() override;
	bool Stop() override;

	ov::StopWatch _stop_watch;
	std::shared_ptr<mon::StreamMetrics> _stream_metrics;
};
