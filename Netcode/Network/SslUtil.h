#pragma once

#include <Netcode/HandleDecl.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <openssl/ssl3.h>
#include <system_error>

namespace Netcode::Network {

	template<typename T>
	struct SslDeleter;

	template<>
	struct SslDeleter<SSL> {
		void operator()(SSL * ctx) const { SSL_clear(ctx); }
	};

	template<>
	struct SslDeleter<SSL_CTX> {
		void operator()(SSL_CTX * ctx) const { SSL_CTX_free(ctx); }
	};

	template<>
	struct SslDeleter<BIO> {
		void operator()(BIO * bio) const { BIO_free_all(bio); }
	};

	template<typename T>
	using ssl_ptr = std::unique_ptr<T, SslDeleter<T>>;

	struct UdpPacket;

	std::error_code SslClientProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio);
	std::error_code SslListenProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio);
	std::error_code SslAcceptProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio);
	std::error_code SslReceive(SSL * ssl, MutableArrayView<uint8_t> & destBuffer, ArrayView<uint8_t> sourceBuffer);
	std::error_code SslReceive(SSL * ssl, UdpPacket * packet);
	std::error_code SslSend(SSL * ssl, MutableArrayView<uint8_t> & destBuffer, ArrayView<uint8_t> sourceBuffer);
	std::error_code SslSend(SSL * ssl, UdpPacket * packet, ArrayView<uint8_t> sourceBuffer);
	std::error_code SslShutdown(SSL * ssl, BIO ** outBio);

	/**
	 * @param dtlsPayloadSize = MTU - IP header - UDP header - DTLS header
	 * @return zero on error
	 */
	uint32_t GetEncryptedPayloadSize(SSL * ssl, uint32_t dtlsPayloadSize);

	std::string Sha256(std::string_view view);
	std::string GenerateNonce();

	void SslInitializeCookies();

	int32_t SslGenerateCookie(SSL * ssl, uint8_t * cookie, uint32_t * cookieLength);
	
	/**
	 * Suitable for a cookie verification callback
	 * @note assumes that the SSL_set_ex_data(0, p) was set with an UdpPacket* instance.
	 */
	int32_t SslVerifyCookie(SSL * ssl, const uint8_t * cookie, uint32_t cookieLength);

	int32_t SslVerifyCertificate(int32_t ok, X509_STORE_CTX * ctx);
	
}
