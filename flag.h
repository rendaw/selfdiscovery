#ifndef FLAG_H
#define FLAG_H

#include <queue>

#include "ren-general/string.h"

class Flag
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

#endif // FLAG_H

