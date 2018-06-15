#pragma once
#include <functional>

#include <Discord/Client.h>
#include <QtCore/QList>

using PermissionCallback = std::function<void(bool)>;

class Permissions {
public:
	static void ContainsPermission(Discord::Client& client, snowflake_t guildId, snowflake_t memberId, unsigned int permissionList, PermissionCallback callback);
	static void MatchesPermission(Discord::Client& client, snowflake_t guildId, snowflake_t memberId, unsigned int requiredPermission, PermissionCallback callback);
};