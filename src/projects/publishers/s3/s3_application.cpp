#include "s3_application.h"

#include "s3_private.h"
#include "s3_publisher.h"
#include "s3_stream.h"

#define S3_PUBLISHER_ERROR_DOMAIN "S3Publisher"

std::shared_ptr<S3Application> S3Application::Create(const std::shared_ptr<pub::Publisher> &publisher, const info::Application &application_info)
{
	auto application = std::make_shared<S3Application>(publisher, application_info);
	application->Start();
	return application;
}

S3Application::S3Application(const std::shared_ptr<pub::Publisher> &publisher, const info::Application &application_info)
	: PushApplication(publisher, application_info)
{
}

S3Application::~S3Applizcation()
{
	Stop();
	logtd("S3Application(%d) has been terminated finally", GetId());
}

bool S3Application::Start()
{
	return Application::Start();
}

bool S3Application::Stop()
{
	return Application::Stop();
}

std::shared_ptr<pub::Stream> S3Application::CreateStream(const std::shared_ptr<info::Stream> &info, uint32_t worker_count)
{
	logtd("CreateStream : %s/%u", info->GetName().CStr(), info->GetId());

	return S3Stream::Create(GetSharedPtrAs<pub::Application>(), *info);
}

bool S3Application::DeleteStream(const std::shared_ptr<info::Stream> &info)
{
	logtd("DeleteStream : %s/%u", info->GetName().CStr(), info->GetId());

	auto stream = std::static_pointer_cast<S3Stream>(GetStream(info->GetId()));
	if (stream == nullptr)
	{
		logte("S3Application::Delete stream failed. Cannot find stream (%s)", info->GetName().CStr());
		return false;
	}

	logtd("%s/%s stream has been deleted", GetName().CStr(), stream->GetName().CStr());

	return true;
}

void S3Application::SessionUpdate(std::shared_ptr<S3Stream> stream, std::shared_ptr<info::Push> userdata)
{
	// If there is no session, create a new file(record) session.
	auto session = std::static_pointer_cast<S3Session>(stream->GetSession(userdata->GetSessionId()));
	if (session == nullptr)
	{
		session = stream->CreateSession();
		if (session == nullptr)
		{
			logte("Could not create session");
			return;
		}
		userdata->SetSessionId(session->GetId());

		session->SetPush(userdata);
	}

	if (userdata->GetEnable() == true && userdata->GetRemove() == false)
	{
		SessionStart(session);
	}

	if (userdata->GetEnable() == false || userdata->GetRemove() == true)
	{
		SessionStop(session);
	}
}

void S3Application::SessionUpdateByStream(std::shared_ptr<S3Stream> stream, bool stopped)
{
	if (stream == nullptr)
	{
		return;
	}

	for (uint32_t i = 0; i < _userdata_sets.GetCount(); i++)
	{
		auto userdata = _userdata_sets.GetAt(i);
		if (userdata == nullptr)
			continue;

		if (userdata->GetStreamName() != stream->GetName())
			continue;

		if (stopped == true)
		{
			userdata->SetState(info::Push::PushState::Ready);
		}
		else
		{
			SessionUpdate(stream, userdata);
		}
	}
}

void S3Application::SessionUpdateByUser()
{
	for (uint32_t i = 0; i < _userdata_sets.GetCount(); i++)
	{
		auto userdata = _userdata_sets.GetAt(i);
		if (userdata == nullptr)
			continue;

		// Find a stream related to Userdata.
		auto stream = std::static_pointer_cast<S3Stream>(GetStream(userdata->GetStreamName()));
		if (stream != nullptr && stream->GetState() == pub::Stream::State::STARTED)
		{
			SessionUpdate(stream, userdata);
		}
		else
		{
			userdata->SetState(info::Push::PushState::Ready);
		}

		if (userdata->GetRemove() == true)
		{
			if (stream != nullptr && userdata->GetSessionId() != 0)
			{
				stream->DeleteSession(userdata->GetSessionId());
			}

			_userdata_sets.DeleteByKey(userdata->GetId());
		}
	}
}

void S3Application::SessionStart(std::shared_ptr<S3Session> session)
{
	// Check the status of the session.
	auto session_state = session->GetState();

	switch (session_state)
	{
		// State of disconnected and ready to connect
		case pub::Session::SessionState::Ready:
			[[fallthrough]];
		// State of stopped
		case pub::Session::SessionState::Stopped:
			[[fallthrough]];
		// State of failed (connection refused, disconnected)
		case pub::Session::SessionState::Error:
			session->Start();
			break;
		// State of Started
		case pub::Session::SessionState::Started:
			[[fallthrough]];
		// State of Stopping
		case pub::Session::SessionState::Stopping:
			break;
	}

	auto next_session_state = session->GetState();
	if (session_state != next_session_state)
	{
		logtd("Changed State. State(%d - %d)", session_state, next_session_state);
	}
}

void S3Application::SessionStop(std::shared_ptr<S3Session> session)
{
	auto session_state = session->GetState();

	switch (session_state)
	{
		case pub::Session::SessionState::Started:
			session->Stop();
			break;
		case pub::Session::SessionState::Ready:
			[[fallthrough]];
		case pub::Session::SessionState::Stopping:
			[[fallthrough]];
		case pub::Session::SessionState::Stopped:
			[[fallthrough]];
		case pub::Session::SessionState::Error:
			break;
	}

	auto next_session_state = session->GetState();
	if (session_state != next_session_state)
	{
		logtd("Changed State. State(%d - %d)", session_state, next_session_state);
	}
}

std::shared_ptr<ov::Error> S3Application::PushStart(const std::shared_ptr<info::Push> &push)
{
	// Validation check for required parameters
	if (push->GetId().IsEmpty() == true || push->GetStreamName().IsEmpty() == true || push->GetUrl().IsEmpty() == true || push->GetProtocol().IsEmpty() == true)
	{
		ov::String error_message = "There is no required parameter [";

		if (push->GetId().IsEmpty() == true)
		{
			error_message += " id";
		}

		if (push->GetStreamName().IsEmpty() == true)
		{
			error_message += " stream.name";
		}

		if (push->GetUrl().IsEmpty() == true)
		{
			error_message += " url";
		}

		if (push->GetProtocol().IsEmpty() == true)
		{
			error_message += " protocol";
		}

		error_message += "]";

		return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::FailureInvalidParameter, error_message);
	}

	// Validation check for dupulicate id
	if (_userdata_sets.GetByKey(push->GetId()) != nullptr)
	{
		ov::String error_message = "Duplicate ID already exists";

		return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::FailureDupulicateKey, error_message);
	}

	// Validation check for protocol scheme
	if (push->GetUrl().HasPrefix("s3://") == false)
	{
		ov::String error_message = "Unsupported protocol";

		return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::FailureInvalidParameter, error_message);
	}

	// Remove suffix '/" of s3 url
	while (push->GetUrl().HasSuffix("/"))
	{
		ov::String tmp_url = push->GetUrl().Substring(0, push->GetUrl().IndexOfRev('/'));
		push->SetUrl(tmp_url);
	}

	push->SetEnable(true);
	push->SetRemove(false);

	_userdata_sets.Set(push);

	SessionUpdateByUser();

	return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::Success, "Success");
}

std::shared_ptr<ov::Error> S3Application::PushStop(const std::shared_ptr<info::Push> &push)
{
	if (push->GetId().IsEmpty() == true)
	{
		ov::String error_message = "There is no required parameter [";

		if (push->GetId().IsEmpty() == true)
		{
			error_message += " id";
		}

		error_message += "]";

		return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::FailureInvalidParameter, error_message);
	}

	auto userdata = _userdata_sets.GetByKey(push->GetId());
	if (userdata == nullptr)
	{
		ov::String error_message = ov::String::FormatString("There is no push information related to the ID [%s]", push->GetId().CStr());

		return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::FailureNotExist, error_message);
	}

	userdata->SetEnable(false);
	userdata->SetRemove(true);

	SessionUpdateByUser();

	return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::Success, "Success");
}

std::shared_ptr<ov::Error> S3Application::GetPushes(const std::shared_ptr<info::Push> push, std::vector<std::shared_ptr<info::Push>> &results)
{
	for (uint32_t i = 0; i < _userdata_sets.GetCount(); i++)
	{
		auto userdata = _userdata_sets.GetAt(i);
		if (userdata == nullptr)
			continue;

		if (!push->GetId().IsEmpty() && push->GetId() != userdata->GetId())
			continue;

		results.push_back(userdata);
	}

	return ov::Error::CreateError(S3_PUBLISHER_ERROR_DOMAIN, ErrorCode::Success, "Success");
}
