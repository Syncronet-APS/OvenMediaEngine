#include "s3_publisher.h"

#include "s3_private.h"

#define UNUSED(expr)  \
	do                \
	{                 \
		(void)(expr); \
	} while (0)

std::shared_ptr<S3Publisher> S3Publisher::Create(const cfg::Server &server_config, const std::shared_ptr<MediaRouteInterface> &router)
{
	auto obj = std::make_shared<S3Publisher>(server_config, router);

	if (!obj->Start())
	{
		logte("An error occurred while creating S3Publisher");
		return nullptr;
	}

	return obj;
}

S3Publisher::S3Publisher(const cfg::Server &server_config, const std::shared_ptr<MediaRouteInterface> &router)
	: Publisher(server_config, router)
{
	logtd("S3Publisher has been create");
}

S3Publisher::~S3Publisher()
{
	logtd("S3Publisher has been terminated finally");
}

bool S3Publisher::Start()
{
	return Publisher::Start();
}

bool S3Publisher::Stop()
{
	return Publisher::Stop();
}

bool S3Publisher::OnCreateHost(const info::Host &host_info)
{
	return true;
}

bool S3Publisher::OnDeleteHost(const info::Host &host_info)
{
	return true;
}

std::shared_ptr<pub::Application> S3Publisher::OnCreatePublisherApplication(const info::Application &application_info)
{
	if (IsModuleAvailable() == false)
	{
		return nullptr;
	}

	return S3Application::Create(S3Publisher::GetSharedPtrAs<pub::Publisher>(), application_info);
}

bool S3Publisher::OnDeletePublisherApplication(const std::shared_ptr<pub::Application> &application)
{
	auto s3_application = std::static_pointer_cast<S3Application>(application);
	if (s3_application == nullptr)
	{
		logte("Could not found file application. app:%s", s3_application->GetName().CStr());
		return false;
	}

	// Applications and child streams must be terminated.

	return true;
}
