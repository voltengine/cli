#include "http.hpp"

namespace fs = std::filesystem;

static std::atomic_uint32_t handle_count = 0;

namespace util {

http::http() noexcept {
	if (handle_count++ == 0)
		curl_global_init(CURL_GLOBAL_DEFAULT);

	handle = curl_easy_init();
	
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_function);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, curl_header_function);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(handle, CURLOPT_BUFFERSIZE, buffer_size);

	tmp_buffer.reserve(buffer_size);
}

http::~http() noexcept {
	curl_easy_cleanup(handle);

	if (--handle_count == 0)
		curl_global_cleanup();
}

void http::send() {
	curl_slist *curl_header = nullptr;
	for (auto &header : request_headers) {
		std::string str = header.first + ": " + header.second;
    	curl_header = curl_slist_append(curl_header, str.c_str());
	}
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, curl_header);

	CURLcode curl_code = curl_easy_perform(handle);

	tmp_response.code = 0;
	tmp_response.headers.clear();

	if (curl_code != CURLE_OK)
		throw http::error(curl_easy_strerror(curl_code));
}

void http::set_version(http::version version) noexcept {
	curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, version);
}

void http::set_method(std::string_view method) noexcept {
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method.data());
}

void http::set_url(std::string_view url) noexcept {
	curl_easy_setopt(handle, CURLOPT_URL, url.data());
}

void http::set_header(const std::string &name, std::string_view value) {
	request_headers[name] = value;
}

void http::set_certificate(const std::filesystem::path &path) {
	std::string str = path.string();
	curl_easy_setopt(handle, CURLOPT_CAINFO, str.c_str());
}

void http::set_timeout(const std::chrono::seconds &duration) noexcept {
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, duration.count());
}

void http::on_response(std::function<void(const response &)> &&callback) noexcept {
	response_callback = std::move(callback);
}

void http::on_data(std::function<void(const buffer &)> &&callback) noexcept {
	data_callback = std::move(callback);
}

size_t http::curl_header_function(char *buffer, size_t size, size_t nitems, void *userdata) {
	size_t actual_size = size * nitems;
	http *http = static_cast<util::http *>(userdata);

	std::string line(buffer, actual_size);
	size_t index = line.find(": ");

	if (index != std::string::npos) {
		std::string name = line.substr(0, index);
		std::string value = line.substr(index + 2, line.size() - index - 4);

		http->tmp_response.headers[name] = value;
	}

	return actual_size;
}

size_t http::curl_write_function(char *ptr, size_t size, size_t nmemb, void *userdata) {
	size_t actual_size = size * nmemb;
	http *http = static_cast<util::http *>(userdata);

	if (http->tmp_response.code == 0) {
		long response_code;
		curl_easy_getinfo(http->handle, CURLINFO_RESPONSE_CODE, &response_code);
		http->tmp_response.code = response_code;

		if (http->response_callback.has_value())
			http->response_callback.value()(http->tmp_response);
	}

	if (http->data_callback.has_value()) {
		http->tmp_buffer.assign(ptr, ptr + actual_size);
		http->data_callback.value()(http->tmp_buffer);
	}

	return actual_size;
}

}
