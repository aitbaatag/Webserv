#include "../../Includes/Http_Req_Res/Request_Struct.hpp"


Request::Request() : method(""), uri(""), path("")
                    , version("") , query(""), fragment(""), field_name("")
    , field_body(""), chunk_size_str(""), chunk_bytes_read(0)
    , chunk_size(0), media_type(""), body_length(0)
    , boundary(""), charset(""), Content_Type("")
    , error_status(0), filename(""), currentHeader("")
    , currentData(""), body_start_pos(0), body_write(0)
  {
  }

Request::~Request()
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

void Request::reset()
{
    if (fileStream.is_open())
        fileStream.close();

    if (!filename.empty()) {
        if (access(filename.c_str(), F_OK) == 0) {
            unlink(filename.c_str());
        }
    }

    method.clear();
    uri.clear();
    path.clear();
    version.clear();
    query.clear();
    fragment.clear();
    headers.clear();
    formData.clear();
    field_name.clear();
    field_body.clear();
    body_length_req = 0;
    chunk_size_str.clear();
    chunk_bytes_read = 0;
    chunk_size = 0;
    media_type.clear();
    body_length = 0;
    boundary.clear();
    charset.clear();
    Content_Type.clear();
    error_status = 0;
    filename.clear();
    currentHeader.clear();
    currentData.clear();
    body_start_pos = 0;
    body_write = 0;
    _client = NULL;
}


