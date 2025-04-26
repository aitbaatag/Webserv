#ifndef SESSION_HPP
#define SESSION_HPP

#include "../libraries/Libraries.hpp"


class Session
{
	private:
		std::string sessionId;
        std::string username;
        std::string note;
        time_t createdAt;
        time_t expiresAt;
        bool valid;
        std::string generateSessionId() const;

	public:
        Session() {}; 
		Session(const std::string &username, const std::string &note);
        ~Session();

        // Getters
        const std::string &getSessionId() const;
        const std::string &getUsername() const;
        const std::string &getNote() const;
        time_t getCreatedAt() const;
        time_t getExpiresAt() const;
        bool isSessionValid() const;

	    // Setters
	    void setNote(const std::string &newNote);
	    void invalidate();
};


#endif