# Wizard's Toolkit

**Wizard's Toolkit** is a collection of command-line utilities and a cross-platform C API for encryption, decryption, and message authentication. Use the utilities to protect and verify files from the shell, or embed the library in your own projects to take advantage of its encryption and hashing methods.

[![Release](https://img.shields.io/badge/release-1.0.9--4-blue)](https://github.com/ImageMagick/WizardsToolkit)
[![License](https://img.shields.io/badge/license-Apache%202.0%20(derived)-green)](#license)

---

## Table of Contents

- [Overview](#overview)
- [Features and Capabilities](#features-and-capabilities)
  - [Command-Line Utilities](#command-line-utilities)
  - [Application Programming Interface (API)](#application-programming-interface-api)
- [Supported Platforms](#supported-platforms)
- [Getting Started](#getting-started)
- [License](#license)
- [Resources](#resources)

---

## Overview

The Wizard's Toolkit provides command-line utilities that let you:

- Convert plaintext to ciphertext
- Convert ciphertext to plaintext
- Report the properties associated with ciphertext
- Compute the message digest of a file
- Read message digests from a file and authenticate them

In addition, the toolkit includes a cross-platform **C API** offering a broad range of encryption and hash methods that many developers will find useful in their own applications.

Wizard's Toolkit is free software. It is delivered as either a ready-to-run binary distribution or as source code that you may use, copy, modify, and distribute in both open-source and proprietary applications. It is distributed under a derived Apache 2.0 license (see [License](#license)).

> **Current release:** Wizard's Toolkit **1.0.9-4**

## Features and Capabilities

### Command-Line Utilities

The command-line utilities cover the most common encryption and integrity-checking tasks:

| Task | Description |
| --- | --- |
| **Encrypt** | Convert plaintext to ciphertext. |
| **Decrypt** | Convert ciphertext back to plaintext. |
| **Inspect** | Report the properties associated with ciphertext. |
| **Digest** | Compute the message digest of a file. |
| **Verify** | Read message digests from a file and verify them against the corresponding data. |

### Application Programming Interface (API)

The C API exposes the same building blocks, along with many supporting facilities, so you can integrate them directly into your software.

#### Cryptography and Authentication

- **Secure hashing:** MD5, SHA-1, SHA-224, SHA-256, SHA-384, and SHA-512
- **Keyed-hash message authentication:** generate a keyed hash (HMAC) to verify the integrity and authenticity of a message
- **Encryption and decryption:** encipher and decipher messages with AES, Serpent, and Twofish
- **User authentication:** authenticate a user (currently, only secret-key authentication is supported)
- **Random numbers:** generate cryptographically strong random numbers
- **Entropy enhancement:** increase entropy through message compression
- **Checksums:** compute a cyclic redundancy check (CRC)

#### File and Data Handling

- Return files as Binary Large Objects (BLOBs)
- Read and write characters, shorts, words, and long words to and from a file
- Map content to its associated MIME type and file extension
- Parse or generate XML documents
- Use convenience methods for working with strings

#### Data Structures

- Store and retrieve key-value pairs using a hashmap or linked list
- Store and retrieve key-value pairs using a self-adjusting binary tree

#### Runtime Support

- Handle C exceptions
- Log events to the console or to an XML-style file
- Allocate and free anonymous memory-mapped memory
- Acquire and relinquish resources
- Acquire and relinquish semaphores

## Supported Platforms

Wizard's Toolkit is cross-platform and runs on:

- Linux
- Windows
- macOS
- iOS
- Android
- ...and others

## Getting Started

You can use Wizard's Toolkit in either of two ways:

1. **Binary distribution:** download a ready-to-run build for your platform from the [project website](https://wizard.imagemagick.org).
2. **Source code:** clone or download the source from the [GitHub repository](https://github.com/ImageMagick/WizardsToolkit) and build it for your platform.

For detailed installation instructions, command-line usage, and API documentation, see the project website.

## License

Wizard's Toolkit is distributed under a derived Apache 2.0 license. You may use, copy, modify, and distribute it in both open and proprietary applications. See the license file included with the distribution for the full terms.

## Resources

- **Website (authoritative):** <https://wizard.imagemagick.org>
- **Source code (authoritative):** <https://github.com/ImageMagick/WizardsToolkit>
