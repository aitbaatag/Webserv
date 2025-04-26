#include "../../Includes/cookies/session_manager.hpp"

SessionManager::SessionManager() {}

SessionManager::~SessionManager()
{
	clearAllSessions();
}

Session &SessionManager::createSession(const std::string &username, const std::string &note)
{
    Session newSession(username, note);
    std::string id = newSession.getSessionId();
    sessions.insert(std::make_pair(id, newSession));
    return sessions.at(id);
}

Session &SessionManager::getSession(const std::string &sessionId)
{
	if (!hasSession(sessionId))
	{
		throw std::runtime_error("Session not found");
	}

	return sessions[sessionId];
}

void SessionManager::removeSession(const std::string &sessionId)
{
	std::map<std::string, Session>::iterator it = sessions.find(sessionId);
	if (it != sessions.end())
	{
		sessions.erase(it);
	}
}

void SessionManager::cleanExpiredSessions()
{
	std::map<std::string, Session>::iterator it = sessions.begin();
	while (it != sessions.end())
	{
		if (!it->second.isSessionValid())
		{
			sessions.erase(it++);
		}
		else
		{ ++it;
		}
	}
}

std::string SessionManager::generateCookie(const std::string &sessionId) const
{
	if (!hasSession(sessionId))
	{
		return "";
	}

	return "sessionId=" + sessionId + "; HttpOnly; Secure";
}

void SessionManager::clearAllSessions()
{
	sessions.clear();
}

bool SessionManager::hasSession(const std::string &sessionId) const
{
	return sessions.find(sessionId) != sessions.end();
}

std::vector<std::string > SessionManager::getActiveSessionIds() const
{
	std::vector<std::string > activeIds;
	std::map<std::string, Session>::const_iterator it;
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		if (it->second.isSessionValid())
		{
			activeIds.push_back(it->first);
		}
	}

	return activeIds;
}