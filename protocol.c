#include "cache.h"
#include "config.h"
#include "protocol.h"

enum protocol_version parse_protocol_version(const char *value)
{
	if (!strcmp(value, "0"))
		return protocol_v0;
	else if (!strcmp(value, "1"))
		return protocol_v1;
	else
		return protocol_unknown_version;
}

enum protocol_version get_protocol_version_config(void)
{
	const char *value;
	if (!git_config_get_string_const("protocol.version", &value)) {
		enum protocol_version version = parse_protocol_version(value);

		if (version == protocol_unknown_version)
			die("unknown value for config 'protocol.version': %s",
			    value);

		return version;
	}

	return protocol_v0;
}

enum protocol_version determine_protocol_version_server(void)
{
	const char *git_protocol = getenv(GIT_PROTOCOL_ENVIRONMENT);
	enum protocol_version version = protocol_v0;

	if (git_protocol) {
		struct string_list list = STRING_LIST_INIT_DUP;
		const struct string_list_item *item;
		string_list_split(&list, git_protocol, ':', -1);

		for_each_string_list_item(item, &list) {
			const char *value;
			enum protocol_version v;

			if (skip_prefix(item->string, "version=", &value)) {
				v = parse_protocol_version(value);
				if (v > version)
					version = v;
			}
		}

		string_list_clear(&list, 0);
	}

	return version;
}

enum protocol_version determine_protocol_version_client(const char *server_response)
{
	enum protocol_version version = protocol_v0;

	if (skip_prefix(server_response, "version ", &server_response)) {
		version = parse_protocol_version(server_response);

		if (version == protocol_unknown_version)
			die("server is speaking an unknown protocol");
		if (version == protocol_v0)
			die("protocol error: server explicitly said version 0");
	}

	return version;
}
