#include "s3_stream.h"

#include <regex>

#include "base/publisher/application.h"
#include "base/publisher/stream.h"
#include "s3_application.h"
#include "s3_private.h"

std::shared_ptr<S3Stream> S3Stream::Create(const std::shared_ptr<pub::Application> application,
										   const info::Stream &info)
{
	auto stream = std::make_shared<S3Stream>(application, info);
	return stream;
}

S3Stream::S3Stream(const std::shared_ptr<pub::Application> application,
				   const info::Stream &info)
	: Stream(application, info)
{
}

S3Stream::~S3Stream()
{
	logtd("S3Stream(%s/%s) has been terminated finally",
		  GetApplicationName(), GetName().CStr());
}

bool S3Stream::Start()
{
	if (GetState() != Stream::State::CREATED)
	{
		return false;
	}

	if (!CreateStreamWorker(2))
	{
		return false;
	}

	logtd("S3Stream(%ld) has been started", GetId());

	std::static_pointer_cast<S3Application>(GetApplication())->SessionUpdateByStream(std::static_pointer_cast<S3Stream>(GetSharedPtr()), false);

	_stop_watch.Start();

	return Stream::Start();
}

bool S3Stream::Stop()
{
	logtd("S3Stream(%u) has been stopped", GetId());

	std::static_pointer_cast<S3Application>(GetApplication())->SessionUpdateByStream(std::static_pointer_cast<S3Stream>(GetSharedPtr()), true);

	if (GetState() != Stream::State::STARTED)
	{
		return false;
	}

	return Stream::Stop();
}

void S3Stream::SendFrame(const std::shared_ptr<MediaPacket> &media_packet)
{
	// Periodically check the session. Retry the session in which the error occurred.
	if (_stop_watch.IsElapsed(5000) && _stop_watch.Update())
	{
		std::static_pointer_cast<S3Application>(GetApplication())->SessionUpdateByStream(std::static_pointer_cast<S3Stream>(GetSharedPtr()), false);
	}

	auto stream_packet = std::make_any<std::shared_ptr<MediaPacket>>(media_packet);

	BroadcastPacket(stream_packet);

	MonitorInstance->IncreaseBytesOut(*pub::Stream::GetSharedPtrAs<info::Stream>(), PublisherType::S3, media_packet->GetData()->GetLength() * GetSessionCount());
}

void S3Stream::SendVideoFrame(const std::shared_ptr<MediaPacket> &media_packet)
{
	if (GetState() != Stream::State::STARTED)
	{
		return;
	}

	SendFrame(media_packet);
}

void S3Stream::SendAudioFrame(const std::shared_ptr<MediaPacket> &media_packet)
{
	if (GetState() != Stream::State::STARTED)
	{
		return;
	}

	SendFrame(media_packet);
}

std::shared_ptr<S3Session> S3Stream::CreateSession()
{
	auto session = S3Session::Create(GetApplication(), GetSharedPtrAs<pub::Stream>(), this->IssueUniqueSessionId());
	if (session == nullptr)
	{
		logte("Internal Error : Cannot create session");
		return nullptr;
	}

	AddSession(session);

	return session;
}

bool S3Stream::DeleteSession(uint32_t session_id)
{
	return RemoveSession(session_id);
}
