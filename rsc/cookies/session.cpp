#include "../../Includes/cookies/session.hpp"

Session::Session(const std::string &username, const std::string &note): username(username), note(note), valid(true)
{
	sessionId = generateSessionId();
	createdAt = time(NULL);
	expiresAt = createdAt + 3600;
}

Session::~Session() {}

std::string Session::generateSessionId() const
{
	std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string uuid;
	srand(time(NULL));

	for (int i = 0; i < 32; ++i)
	{
		uuid += chars[rand() % chars.length()];
	}

	return uuid;
}

// Getters
const std::string &Session::getSessionId() const
{
	return sessionId;
}

const std::string &Session::getUsername() const
{
	return username;
}

const std::string &Session::getNote() const
{
	return note;
}

time_t Session::getCreatedAt() const
{
	return createdAt;
}

time_t Session::getExpiresAt() const
{
	return expiresAt;
}

bool Session::isSessionValid() const
{
	return valid && (time(NULL) < expiresAt);
}

// Setters
void Session::setNote(const std::string &newNote)
{
	note = newNote;
}

void Session::invalidate()
{
	valid = false;
}