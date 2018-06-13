#include <node.h>
#include <nan.h>
#include <v8.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>

using namespace v8;
using namespace node;

static RSA *generateKey(int num, unsigned long e)
{
#if OPENSSL_VERSION_NUMBER < 0x009080001
	return RSA_generate_key(num, e, NULL, NULL);
#else
	BIGNUM *eBig = BN_new();

	if (eBig == NULL)
	{
		return NULL;
	}

	if (!BN_set_word(eBig, e))
	{
		BN_free(eBig);
		return NULL;
	}

	RSA *result = RSA_new();

	if (result == NULL)
	{
		BN_free(eBig);
		return NULL;
	}

	if (RSA_generate_key_ex(result, num, eBig, NULL) < 0)
	{
		RSA_free(result);
		result = NULL;
	}

	BN_free(eBig);

	return result;
#endif
}

static Nan::MaybeLocal<Object> toBuffer(BIO *bio)
{
	char *data;
	long length = BIO_get_mem_data(bio, &data);

	return Nan::CopyBuffer(data, length);
}

void Generate(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
	Nan::HandleScope scope;

	int modulusBits = 2048;
	int exponent = 65537;
	int passPhraseLength = 0;
	char *passPhrase = NULL;
	const EVP_CIPHER *cipher = NULL;

	if (info[0]->IsInt32())
	{
		modulusBits = Nan::To<v8::Int32>(info[0]).ToLocalChecked()->Value();
	}

	if (info[1]->IsInt32())
	{
		exponent = Nan::To<v8::Int32>(info[1]).ToLocalChecked()->Value();
	}

	if (info[2]->IsString())
	{
		v8::Local<v8::String> passPhraseStr = info[2]->ToString();
		passPhraseLength = passPhraseStr->Length();
		passPhrase = new char[passPhraseLength + 1];
		passPhraseLength = passPhraseStr->WriteOneByte(reinterpret_cast<unsigned char *>(passPhrase));
		cipher = (EVP_CIPHER *)EVP_des_ede3_cbc();
	}

	if (modulusBits < 512)
	{
		Nan::ThrowTypeError("Expected modulus bit count bigger than 512.");
		return;
	}

	if (exponent < 0)
	{
		Nan::ThrowTypeError("Expected positive exponent.");
		return;
	}

	if ((exponent & 1) == 0)
	{
		Nan::ThrowTypeError("Expected odd exponent.");
		return;
	}

	RSA *rsa = generateKey(modulusBits, (unsigned int)exponent);

	if (!rsa)
	{
		Nan::ThrowError("Failed creating RSA context.");
		return;
	}

	BIO *publicBio = BIO_new(BIO_s_mem());
	BIO *privateBio = BIO_new(BIO_s_mem());

	if (!publicBio || !privateBio)
	{
		if (publicBio)
		{
			BIO_vfree(publicBio);
		}

		if (privateBio)
		{
			BIO_vfree(privateBio);
		}

		RSA_free(rsa);

		Nan::ThrowError("Failed to allocate OpenSSL buffers.");
		return;
	}

	if (!PEM_write_bio_RSA_PUBKEY(publicBio, rsa))
	{
		BIO_vfree(publicBio);
		BIO_vfree(privateBio);
		RSA_free(rsa);

		Nan::ThrowError("Failed exporting public key.");
		return;
	}

	if (!PEM_write_bio_RSAPrivateKey(privateBio, rsa, cipher, (unsigned char *)passPhrase, passPhraseLength, NULL, NULL))
	{
		BIO_vfree(publicBio);
		BIO_vfree(privateBio);
		RSA_free(rsa);
		if (NULL != passPhrase)
		{
			delete[] passPhrase;
		}

		Nan::ThrowError("Failed exporting private key.");
		return;
	}

	Nan::MaybeLocal<Object> publicKey = toBuffer(publicBio);
	Nan::MaybeLocal<Object> privateKey = toBuffer(privateBio);

	BIO_vfree(publicBio);
	BIO_vfree(privateBio);
	RSA_free(rsa);
	if (NULL != passPhrase)
	{
		delete[] passPhrase;
	}

	Local<Object> result = Nan::New<Object>();

	Nan::Set(result, Nan::New<String>("publicKey").ToLocalChecked(), publicKey.ToLocalChecked());
	Nan::Set(result, Nan::New<String>("privateKey").ToLocalChecked(), privateKey.ToLocalChecked());

	info.GetReturnValue().Set(result);
}

void InitAll(Handle<Object> exports)
{
	Nan::Set(exports, Nan::New<String>("generate").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Generate)).ToLocalChecked());
}

NODE_MODULE(rsa_keypair, InitAll)