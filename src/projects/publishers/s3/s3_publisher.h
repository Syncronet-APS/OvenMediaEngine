#pragma once

#include <orchestrator/orchestrator.h>

#include "base/common_types.h"
#include "base/info/push.h"
#include "base/mediarouter/mediarouter_application_interface.h"
#include "base/ovlibrary/url.h"
#include "base/publisher/publisher.h"
#include "s3_application.h"
#include "s3_userdata.h"

class S3Publisher : public pub::Publisher
{
public:
	static std::shared_ptr<S3Publisher> Create(const cfg::Server &server_config, const std::shared_ptr<MediaRouteInterface> &router);

	S3Publisher(const cfg::Server &server_config, const std::shared_ptr<MediaRouteInterface> &router);
	~S3Publisher() override;
	bool Stop() override;

private:
	bool Start() override;

	//--------------------------------------------------------------------
	// Implementation of Publisher
	//--------------------------------------------------------------------
	PublisherType GetPublisherType() const override
	{
		return PublisherType::S3;
	}
	const char *GetPublisherName() const override
	{
		return "S3Publisher";
	}

	bool OnCreateHost(const info::Host &host_info) override;
	bool OnDeleteHost(const info::Host &host_info) override;
	std::shared_ptr<pub::Application> OnCreatePublisherApplication(const info::Application &application_info) override;
	bool OnDeletePublisherApplication(const std::shared_ptr<pub::Application> &application) override;
};
