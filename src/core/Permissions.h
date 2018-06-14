#pragma once

#include <Discord/Client.h>
#include <QList>

class Permissions {
public:
	static bool ContainsPermission(unsigned int permissions, unsigned int requiredPermission);
	static bool MatchesPermission(unsigned int permissions, unsigned int requiredPermission);
};