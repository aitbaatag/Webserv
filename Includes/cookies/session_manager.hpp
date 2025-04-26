#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include "session.hpp"

class SessionManager
{
	private:
		std::map<std::string, Session> sessions;

	public:
		SessionManager();
        ~SessionManager();

        // Session management
        Session &createSession(const std::string &username, const std::string &note);
        Session &getSession(const std::string &sessionId);
        void removeSession(const std::string &sessionId);
        void cleanExpiredSessions();
        std::string generateCookie(const std::string &sessionId) const;

        // Helper methods
        void clearAllSessions();
        bool hasSession(const std::string &sessionId) const;
        std::vector<std::string > getActiveSessionIds() const;
};


#endif