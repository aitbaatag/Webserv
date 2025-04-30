#include "../../Includes/Http_Req_Res/Request_Struct.hpp"
#include "../../Includes/utlis/utils.hpp"

Request::Request() : method(""), uri(""), path("")
                    , version("") , query(""), fragment(""), field_name("")
    , field_body(""), chunk_size_str(""), chunk_bytes_read(0)
    , chunk_size(0), media_type(""), body_length(0)
    , boundary(""), charset(""), Content_Type("")
    , error_status(0), filename(""), currentHeader("")
    , currentData(""), body_start_pos(0), body_write(0)
  {
    fd_file = -1;
  }

Request::~Request()
{
    if (fd_file > 0)
    { 
      close(fd_file);
      fd_file = -1;
    }
    
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

  close_fd(fd_file);
    // if (fileStream.is_open())
    //     fileStream.close();

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


