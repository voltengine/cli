#include "crypto.cpp"

namespace util {

std::string sha256(std::string_view str) {
	std::vector<unsigned char> buffer;
	buffer.resize(crypto_hash_sha256_BYTES);
	crypto_hash_sha256(
			buffer.data(),
			reinterpret_cast<const unsigned char *>(str.data()),
			str.size());

	std::string hex;
	hex.resize(65);
	sodium_bin2hex(
			hex.data(), hex.size(),
			buffer.data(), buffer.size());
	hex.resize(64);

	return hex;
}

}
