#include "SslUtil.h"
#include "NetworkErrorCode.h"
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <Netcode/Logger.h>
#include "MtuValue.hpp"
#include "BasicPacket.hpp"
#include <Netcode/Utility.h>

namespace Netcode::Network {

	std::error_code SslClientProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio) {
		BIO * rbio = SSL_get_rbio(ssl);

		if(rbio == nullptr) {
			if(receivedPacket != nullptr) {
				rbio = BIO_new_mem_buf(receivedPacket->GetData(), receivedPacket->GetSize());
			} else {
				int zero = 0;
				rbio = BIO_new_mem_buf(&zero, 0);
			}
		} else {
			BUF_MEM bm;
			bm.data = reinterpret_cast<char *>(receivedPacket->GetData());
			bm.flags = 0;
			bm.length = receivedPacket->GetSize();
			bm.max = receivedPacket->GetCapacity();
			BIO_set_mem_buf(rbio, &bm, BIO_NOCLOSE);
		}

		BIO * wbio = SSL_get_wbio(ssl);
		if(wbio == nullptr) {
			wbio = BIO_new(BIO_s_mem());
		}

		SSL_set_bio(ssl, rbio, wbio);
		
		const int connResult = SSL_connect(ssl);

		BUF_MEM * output = nullptr;
		BIO_get_mem_ptr(wbio, &output);

		if(connResult == 1 && output->data == nullptr) {
			return std::error_code{};
		}

		if(connResult == -1 &&
			output->data != nullptr) {
			BIO_up_ref(wbio);
			*outBio = wbio;
			SSL_set_bio(ssl, rbio, nullptr);
			return make_error_code(NetworkErrc::SSL_CONTINUATION_NEEDED);
		}

		return make_error_code(NetworkErrc::SSL_ERROR);
	}

	std::error_code SslListenProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio)
	{
		SSL_clear(ssl);
		SSL_set_options(ssl, SSL_OP_COOKIE_EXCHANGE);
		SSL_set_accept_state(ssl);
		SSL_set_mtu(ssl, 1280);

		BIO * rbio = SSL_get_rbio(ssl);
		if(rbio == nullptr) {
			rbio = BIO_new_mem_buf(receivedPacket->GetData(), receivedPacket->GetSize());
		} else {
			BUF_MEM bm;
			bm.data = reinterpret_cast<char *>(receivedPacket->GetData());
			bm.flags = 0;
			bm.length = receivedPacket->GetSize();
			bm.max = receivedPacket->GetCapacity();
			BIO_set_mem_buf(rbio, &bm, BIO_NOCLOSE);
		}

		BIO * wbio = SSL_get_wbio(ssl);
		if(wbio == nullptr) {
			wbio = BIO_new(BIO_s_mem());
		}

		BIO_set_mem_eof_return(rbio, -1);
		BIO_set_mem_eof_return(wbio, -1);

		SSL_set_bio(ssl, rbio, wbio);

		// DTLS listen will try to zero out the 2nd param on failure...
		sockaddr_storage dummy = {};
		int lstn = DTLSv1_listen(ssl, reinterpret_cast<BIO_ADDR *>(&dummy));

		const size_t pendingBytes = BIO_ctrl_pending(wbio);

		if(lstn == 1 && pendingBytes == 0) {
			return std::error_code{};
		}

		if(lstn == 0 && pendingBytes > 0) {
			BUF_MEM * wb = nullptr;
			BIO_get_mem_ptr(wbio, &wb);
			BIO_up_ref(wbio);
			*outBio = wbio;
			SSL_set_bio(ssl, rbio, nullptr);
			return make_error_code(NetworkErrc::SSL_CONTINUATION_NEEDED);
		}

		return make_error_code(NetworkErrc::SSL_ERROR);
	}

	std::error_code SslReceive(SSL * ssl, MutableArrayView<uint8_t> & destBuffer, ArrayView<uint8_t> sourceBuffer) {
		BIO * rbio = SSL_get_rbio(ssl);
		if(rbio == nullptr) {
			Log::Debug("Recv: SSL_get_rbio() returned null");
			return make_error_code(NetworkErrc::SSL_ERROR);
		}

		BUF_MEM bm;
		bm.data = reinterpret_cast<char *>(const_cast<uint8_t *>(sourceBuffer.Data()));
		bm.flags = 0;
		bm.length = sourceBuffer.Size();
		bm.max = sourceBuffer.Size();
		BIO_set_mem_buf(rbio, &bm, BIO_NOCLOSE);

		SSL_set_bio(ssl, rbio, SSL_get_wbio(ssl));

		int read = SSL_read(ssl, destBuffer.Data(), destBuffer.Size());

		if(read > 0) {
			destBuffer = MutableArrayView<uint8_t>{ destBuffer.Data(), static_cast<size_t>(read) };
			return std::error_code{};
		}
		int rv = SSL_get_error(ssl, 0);

		return make_error_code(NetworkErrc::SSL_ERROR);
	}

	std::error_code SslReceive(SSL * ssl, UdpPacket * packet)
	{
		MutableArrayView<uint8_t> dst{
			packet->GetData(),
			packet->GetCapacity()
		};

		ArrayView<uint8_t> src{
			packet->GetData(),
			packet->GetSize()
		};

		const std::error_code ec = SslReceive(ssl, dst, src);
		
		if(!ec) {
			packet->SetSize(static_cast<uint32_t>(dst.Size()));
		}

		return ec;
	}

	std::error_code SslSend(SSL * ssl, UdpPacket * packet, ArrayView<uint8_t> sourceBuffer)
	{
		MutableArrayView<uint8_t> dst{
			packet->GetData(),
			packet->GetCapacity()
		};

		const std::error_code ec = SslSend(ssl, dst, sourceBuffer);

		if(!ec) {
			packet->SetSize(static_cast<uint32_t>(dst.Size()));
		}

		return ec;
	}

	std::error_code SslSend(SSL * ssl, MutableArrayView<uint8_t> & destBuffer, ArrayView<uint8_t> sourceBuffer) {
		BIO * wbio = SSL_get_wbio(ssl);

		if(wbio == nullptr) {
			wbio = BIO_new(BIO_s_mem());
		}

		BUF_MEM bm;
		bm.max = destBuffer.Size();
		bm.flags = 0;
		bm.length = 0;
		bm.data = reinterpret_cast<char *>(destBuffer.Data());
		BIO_set_mem_buf(wbio, &bm, BIO_NOCLOSE);

		SSL_set_bio(ssl, SSL_get_rbio(ssl), wbio);

		int written = SSL_write(ssl, sourceBuffer.Data(), sourceBuffer.Size());

		BUF_MEM * dst = nullptr;
		BIO_get_mem_ptr(wbio, &dst);

		if(written > 0) {
			destBuffer = MutableArrayView<uint8_t>{ destBuffer.Data(), static_cast<size_t>(dst->length) };
			return std::error_code{};
		}

		return make_error_code(NetworkErrc::SSL_ERROR);
	}

	std::error_code SslShutdown(SSL * ssl, BIO ** outBio)
	{
		BIO * wbio = SSL_get_wbio(ssl);

		if(wbio == nullptr) {
			wbio = BIO_new(BIO_s_mem());
		}

		SSL_set_bio(ssl, SSL_get_rbio(ssl), wbio);

		int rv = SSL_shutdown(ssl);

		std::string s;
		s.resize(1024);

		BUF_MEM bm;
		bm.flags = 0;
		bm.data = s.data();
		bm.length = 0;
		bm.max = s.capacity();
		BIO_set_mem_buf(wbio, &bm, BIO_NOCLOSE);

		ERR_print_errors(wbio);

		Log::Debug("Err: {0}", s);

		BUF_MEM * dst = nullptr;
		BIO_get_mem_ptr(wbio, &dst);

		if(dst->data != nullptr) {
			BIO_up_ref(wbio);
			*outBio = wbio;
			SSL_set_bio(ssl, SSL_get_rbio(ssl), nullptr);
			return make_error_code(NetworkErrc::SSL_CONTINUATION_NEEDED);
		}

		return std::error_code{};
	}

	uint32_t GetEncryptedPayloadSize(SSL * ssl, uint32_t dtlsPayloadSize)
	{
		/*
		 * This is a fallback function as DTLS_get_data_mtu(...) is returning
		 * constant 219. Probably because there is no underlying socket BIO.
		 * 
		 * Only handles the 2 used ciphers.
		 */
		
		const SSL_CIPHER * cipher = SSL_get_current_cipher(ssl);

		if(cipher == nullptr) {
			return 0;
		}
		
		const uint32_t cipherId = SSL_CIPHER_get_id(cipher);

		if(cipherId == TLS1_CK_ECDHE_RSA_WITH_NULL_SHA) {
			constexpr uint32_t overhead = 20;
			
			if(dtlsPayloadSize < overhead) {
				return 0;
			}
			
			return dtlsPayloadSize - overhead;
		}

		if(cipherId == TLS1_CK_ECDHE_RSA_WITH_AES_128_GCM_SHA256) {
			constexpr uint32_t overhead = EVP_GCM_TLS_TAG_LEN + EVP_GCM_TLS_EXPLICIT_IV_LEN;

			if(dtlsPayloadSize < overhead) {
				return 0;
			}
			
			return dtlsPayloadSize - overhead;
		}
		
		return 0;
	}

	std::string Sha256(std::string_view view)
	{
		EVP_MD_CTX * ctx = EVP_MD_CTX_new();

		if(ctx == nullptr)
			return std::string{};

		if(EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1)
			return std::string{};

		if(EVP_DigestUpdate(ctx, view.data(), view.size()) != 1)
			return std::string{};

		uint8_t hash[EVP_MAX_MD_SIZE];
		uint32_t len = 0;

		if(EVP_DigestFinal_ex(ctx, hash, &len) != 1)
			return std::string{};

		std::ostringstream oss;
		for(uint32_t i = 0; i < len; i++) {
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
		}

		EVP_MD_CTX_free(ctx);
		
		return oss.str();
	}

	std::string GenerateNonce()
	{
		std::string randBytes;
		randBytes.resize(33);
		if(RAND_bytes(reinterpret_cast<uint8_t *>(randBytes.data()), 32) == -1) {
			return std::string{};
		}
		return Sha256(randBytes);
	}

	constexpr static uint32_t NUM_SECRETS = 8;

	struct CookieSecret {
		uint8_t data[8];

		CookieSecret() { }
	};

	static std::unique_ptr<CookieSecret[]> cookieSecrets{ nullptr };

	static ArrayView<uint8_t> GetSecret() {
		return ArrayView<uint8_t>{ cookieSecrets[rand() % NUM_SECRETS].data, sizeof(CookieSecret::data)  };
	}
	
	void SslInitializeCookies() {
		cookieSecrets = std::make_unique<CookieSecret[]>(NUM_SECRETS);

		srand(time(0));

		for(uint32_t i = 0; i < NUM_SECRETS; i++) {
			int res;
			do {
				res = RAND_bytes(cookieSecrets[i].data, sizeof(CookieSecret::data));
			} while(res == 0);

			if(res == -1) {
				Log::Error("Failed to generate random number.");
			}
		}
	}

	int32_t SslGenerateCookie(SSL * ssl, uint8_t * cookie, uint32_t * cookieLength) {
		const UdpPacket * pkt = reinterpret_cast<const UdpPacket *>(SSL_get_ex_data(ssl, 0));

		if(pkt == nullptr) {
			return -1;
		}

		UdpEndpoint ep = pkt->GetEndpoint();
		ArrayView<uint8_t> secret = GetSecret();
		if(HMAC(EVP_sha256(), secret.Data(), secret.Size(),
			reinterpret_cast<const unsigned char *>(&ep), ep.size(),
			cookie, cookieLength) == nullptr) {
			return -1;
		}

		return 1;
	}

	int32_t SslVerifyCookie(SSL * ssl, const uint8_t * cookie, uint32_t cookieLength) {
		const UdpPacket * pkt = reinterpret_cast<const UdpPacket *>(SSL_get_ex_data(ssl, 0));

		if(pkt == nullptr) {
			return -1;
		}

		UdpEndpoint ep = pkt->GetEndpoint();

		uint8_t hmac[EVP_MAX_MD_SIZE];

		for(uint32_t i = 0; i < NUM_SECRETS; i++) {
			uint32_t hmacLen = EVP_MAX_MD_SIZE;
			if(HMAC(EVP_sha256(), cookieSecrets[i].data, sizeof(CookieSecret::data),
				reinterpret_cast<const unsigned char *>(&ep), ep.size(),
				hmac, &hmacLen) == nullptr) {
				return -1;
			}

			if(hmacLen == cookieLength && memcmp(hmac, cookie, cookieLength) == 0) {
				return 1;
			}
		}

		return 0;
	}

	int32_t SslVerifyCertificate(int32_t ok, X509_STORE_CTX * ctx) {
		return 1;
	}

	std::error_code SslAcceptProceed(SSL * ssl, UdpPacket * receivedPacket, BIO ** outBio)
	{
		BIO * rbio = SSL_get_rbio(ssl);
		if(rbio == nullptr) {
			Log::Debug("SSL_get_rbio() returned null");
			return make_error_code(NetworkErrc::SSL_ERROR);
		}

		BUF_MEM bm;
		bm.data = reinterpret_cast<char *>(receivedPacket->GetData());
		bm.flags = 0;
		bm.length = receivedPacket->GetSize();
		bm.max = receivedPacket->GetCapacity();
		BIO_set_mem_buf(rbio, &bm, BIO_NOCLOSE);

		BIO * wbio = SSL_get_wbio(ssl);
		if(wbio == nullptr) {
			wbio = BIO_new(BIO_s_mem());
		}

		SSL_set_bio(ssl, rbio, wbio);
		SSL_set_mtu(ssl, MtuValue(1280).GetUdpPayloadSize(receivedPacket->endpoint.address()));

		int accept = SSL_accept(ssl);

		if(accept == -1) {
			int err = SSL_get_error(ssl, accept);
			Log::Debug("?{0}", err);
		}

		BUF_MEM * wb;
		BIO_get_mem_ptr(wbio, &wb);

		if(wb->data != nullptr) {
			BIO_up_ref(wbio);
			*outBio = wbio;
			SSL_set_bio(ssl, rbio, nullptr);
			if(accept == -1) {
				return make_error_code(NetworkErrc::SSL_CONTINUATION_NEEDED);
			}
		}

		if(accept == 1) {
			return std::error_code{};
		}

		Log::Debug("Unexpected SSL flow");
		return make_error_code(NetworkErrc::SSL_ERROR);
	}
	
}
