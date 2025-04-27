#ifndef REQUEST_STRUCT_HPP
#define REQUEST_STRUCT_HPP
#include "../libraries/Libraries.hpp"

class HttpClient;

struct Request
{
	std::string method;
	std::string uri;
	std::string path;
	std::string version;
	std::string query;	// the query string of the request like ?name=ahmed
	std::string fragment;	// the fragment of the request like #section1
	std::map<std::string, std::string > headers;
	std::map<std::string, std::string > formData;
	std::string field_name;
	std::string field_body;
  size_t body_length_req;
  
	// chunked body
	std::string chunk_size_str;
	size_t chunk_bytes_read;
	size_t chunk_size;
	std::string media_type;
	size_t body_length;
	std::string boundary;
	std::string charset;
	std::string Content_Type;
	int error_status;	// the error status of the request
	std::string filename;
	std::string currentHeader;
	std::string currentData;
	std::fstream fileStream;
	size_t body_start_pos;
	size_t body_write;
  HttpClient *_client;

	// Constructor to initialize all members
  Request() : 
    method("")
    , uri("")
    , path("")
    , version("")
    , query("")
    , fragment("")
    , field_name("")
    , field_body("")
    , chunk_size_str("")
    , chunk_bytes_read(0)
    , chunk_size(0)
    , media_type("")
    , body_length(0)
    , boundary("")
    , charset("")
    , Content_Type("")
    , error_status(0)
    , filename("")
    , currentHeader("")
    , currentData("")
    , body_start_pos(0)
    , body_write(0)
  {
  }
  ~Request()
  {
    if (fileStream.is_open())
      fileStream.close();
    
      if (!filename.empty())
      {
        if (access(filename.c_str(), F_OK) == 0)
        {
          unlink(filename.c_str());
        }
	}
}
};


#endif