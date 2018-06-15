#include "Permissions.h"
#include "UmikoBot.h"

void Permissions::ContainsPermission(Discord::Client& client, snowflake_t guildId, snowflake_t memberId, unsigned int permissionList, PermissionCallback callback)
{
	client.getGuildMember(guildId, memberId).then(
		[&client, guildId, memberId, permissionList, callback](const Discord::GuildMember& member)
	{
		unsigned int totalPermissions = 0;
		for (const Discord::Role& role : reinterpret_cast<UmikoBot*>(&client)->GetRoles(guildId))
			for (snowflake_t roleId : member.roles())
				if (roleId == role.id()) 
				{
					totalPermissions |= role.permissions();
					break;
				}

		int x = 1;
		while (x <= permissionList)
		{
			unsigned int currentPermission = permissionList & x;
			if (totalPermissions & currentPermission != 0)
				return callback(true);
			x = x << 1;
		}
		return callback(false);
	});
}

void Permissions::MatchesPermission(Discord::Client& client, snowflake_t guildId, snowflake_t memberId, unsigned int requiredPermission, PermissionCallback callback)
{
	client.getGuildMember(guildId, memberId).then(
		[&client, guildId, memberId, requiredPermission, callback](const Discord::GuildMember& member)
	{
		unsigned int totalPermissions = 0;
		for (const Discord::Role& role : reinterpret_cast<UmikoBot*>(&client)->GetRoles(guildId))
			for (snowflake_t roleId : member.roles())
				if (roleId == role.id())
				{
					totalPermissions |= role.permissions();
					break;
				}

		if (totalPermissions & requiredPermission == requiredPermission)
			return callback(true);

		return callback(true);
	});
}
