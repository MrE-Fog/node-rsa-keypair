# node-rsa-keypair

[![NPM version][npm-image]][npm-url] [![Build Status][travis-image]][travis-url] [![Dependency Status][daviddm-image]][daviddm-url]

Generates a RSA keypair using native OpenSSL library.

This is a fork of [rsa-keygen](https://github.com/sunjith/node-rsa-keygen) with support for encrypting the generated private key with a given pass phrase. The dependencies have also been updated as per the pull request [Update deps](https://github.com/sunjith/node-rsa-keygen/pull/6) by [omsmith](https://github.com/omsmith) and [calvinmetcalf](https://github.com/calvinmetcalf) which was not merged in to the original rsa-keygen.

This code is loosely based on [ursa](https://github.com/Medium/ursa) RSA generation code.

Thanks to all the developers who have contributed to the above projects.

## History

As from node 0.11 the `crypto` library has `publicEncrypt` and `privateDecrypt` functions, we don't need to rely on any external libraries for public-key cryptography.

## Usage

Install the library using npm:

```sh
npm install --save rsa-keypair
```

Or install using yarn:

```sh
yarn add rsa-keypair
```

Use in your code:

```javascript
var rsaKeyPair = require("rsa-keypair");
var keys = rsaKeyPair.generate();
```

## Examples

```javascript
var crypto = require("crypto");
var rsaKeyPair = require("rsa-keypair");

var keys = rsaKeyPair.generate();

var result = crypto.publicEncrypt(
  {
    key: keys.publicKey
  },
  new Buffer("Hello world!")
);
// <Crypted Buffer>

var plaintext = crypto.privateDecrypt(
  {
    key: keys.privateKey
  },
  result
);
// Hello world!
```

```javascript
var crypto = require("crypto");
var rsaKeyPair = require("rsa-keypair");

var keys = rsaKeyPair.generate(4096, 65537, "top secret");
// Generates a 4096-bit RSA key pair with "top secret" as the pass phrase to encrypt the private key

var result = crypto.privateEncrypt(
  {
    key: keys.privateKey,
    passphrase: "top secret",
    padding: crypto.constants.RSA_PKCS1_PADDING
  },
  new Buffer("Hello world!")
);
// <Crypted Buffer>

var plaintext = crypto.publicDecrypt(
  {
    key: keys.publicKey,
    padding: crypto.constants.RSA_PKCS1_PADDING
  },
  result
);
// Hello world!
```

```javascript
var rsaKeyPair = require("rsa-keypair");

var keys = rsaKeyPair.generate(4096, 65537, "top secret");
// Generates a 4096-bit RSA key pair with "top secret" as the pass phrase to encrypt the private key

var publicKeyStr = keys.publicKey.toString();
// The public key string in PEM format which may be written to a file

var privateKeyStr = keys.privateKey.toString();
// The encrypted private key in PEM format which may be written to a file
```

[npm-image]: https://badge.fury.io/js/rsa-keypair.svg
[npm-url]: https://npmjs.org/package/rsa-keypair
[travis-image]: https://travis-ci.com/sunjith/node-rsa-keypair.svg?branch=master
[travis-url]: https://travis-ci.com/sunjith/node-rsa-keypair
[daviddm-image]: https://david-dm.org/sunjith/node-rsa-keypair.svg?theme=shields.io
[daviddm-url]: https://david-dm.org/sunjith/node-rsa-keypair
