/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit cipher content methods.
*/
#ifndef WIZARDSTOOLKIT_UTILITIES_CONTENT_H
#define WIZARDSTOOLKIT_UTILITIES_CONTENT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define CipherProtocolMajor  1
#define CipherProtocolMinor  0 

typedef struct _ContentInfo
{
  BlobInfo
    *plainblob,
    *cipherblob;

  char
    *content,
    *properties;

  CipherInfo
    *cipher_info;

  CipherType
    cipher;

  CipherMode
    mode;

  char
    *nonce;

  AuthenticateInfo
    *authenticate_info;

  AuthenticateMethod
    authenticate_method;

  HashType
    key_hash;

  char
    *keyring,
    *id,
    *passphrase;

  unsigned int
    key_length;

  EntropyInfo
    *entropy_info;

  EntropyType
    entropy;

  unsigned int
    level;

  HMACInfo
    *hmac_info;

  HashType
    hmac;

  RandomInfo
    *random_info;

  HashType
    random_hash;

  size_t
    chunksize;

  time_t
    access_date,
    modify_date,
    create_date,
    timestamp;

  char
    *version;

  unsigned int
    protocol_major,
    protocol_minor;

  size_t
    signature;
} ContentInfo;

extern WizardExport ContentInfo
  *AcquireContentInfo(void),
  *DestroyContentInfo(ContentInfo *);

extern WizardExport WizardBooleanType
  GetContentInfo(ContentInfo *,BlobInfo *,ExceptionInfo *),
  PrintCipherProperties(const ContentInfo *,FILE *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
