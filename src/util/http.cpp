#include "http.hpp"

namespace fs = std::filesystem;

static std::atomic_uint32_t handle_count = 0;

namespace util {

http::http() {
	if (handle_count++ == 0)
		curl_global_init(CURL_GLOBAL_DEFAULT);

	handle = curl_easy_init();
	
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_function);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, curl_header_function);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(handle, CURLOPT_BUFFERSIZE, buffer_size);

	static const std::string agent = std::string("curl/")
			+ curl_version_info(CURLVERSION_NOW)->version;
	curl_easy_setopt(handle, CURLOPT_USERAGENT, agent.c_str());

	tmp_buffer.reserve(buffer_size);
}

http::~http() {
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

	finish_response();
	tmp_response = response();

	if (curl_code != CURLE_OK)
		throw http::error(curl_easy_strerror(curl_code) + std::string("."));
}

void http::set_version(http::version version) {
	curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, version);
}

void http::set_certificate(const std::filesystem::path &path) {
	curl_easy_setopt(handle, CURLOPT_CAINFO, (certificate = path.string()).data());
}

void http::set_method(std::string_view method) {
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, (this->method = method).data());
}

void http::set_url(std::string_view url) {
	curl_easy_setopt(handle, CURLOPT_URL, (this->url = url).data());
}

void http::set_header(const std::string &name, std::string_view value) {
	request_headers[name] = value;
}

void http::remove_header(const std::string &name) {
	request_headers.erase(name);
}

void http::set_body(std::string_view body) {
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, (this->body = body).data());
}

void http::set_timeout(const std::chrono::seconds &duration) {
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

	http &request = *static_cast<util::http *>(userdata);
	request.finish_response();

	if (request.data_callback.has_value()) {
		request.tmp_buffer.assign(ptr, ptr + actual_size);
		request.data_callback.value()(request.tmp_buffer);
	}

	return actual_size;
}

void http::finish_response() {
	if (tmp_response.status == -1) {
		long response_code;
		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);

		if (response_code != 0) {
			tmp_response.status = response_code;

			if (response_callback.has_value())
				response_callback.value()(tmp_response);
		}
	}
}

}
