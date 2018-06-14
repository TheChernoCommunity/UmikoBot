#include "Permissions.h"

bool Permissions::ContainsPermission(unsigned int permissions, unsigned int requiredPermission)
{
	int x = 1;
	while (x < requiredPermission)
	{
		unsigned int currentPermission = requiredPermission & x;
		if (permissions & currentPermission != 0)
			return true;
		x = x << 1;
	}
	return false;
}

bool Permissions::MatchesPermission(unsigned int permissions, unsigned int requiredPermission)
{
	if (permissions & requiredPermission == requiredPermission)
		return true;

	return false;
}
