/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                CCCC   OOO   N   N  TTTTT  EEEEE  N   N  TTTTT               %
%               C      O   O  NN  N    T    E      NN  N    T                 %
%               C      O   O  N N N    T    EEE    N N N    T                 %
%               C      O   O  N  NN    T    E      N  NN    T                 %
%                CCCC   OOO   N   N    T    EEEEE  N   N    T                 %
%                                                                             %
%                      Wizard's Toolkit Content Methods                       %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "wizard/studio.h"
#include "wizard/WizardsToolkit.h"
#include "wizard/blob-private.h"
#include "wizard/exception-private.h"
#include "wizard/string-private.h"
#include "content.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C i p h e r C o n t e n t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireContentInfo() allocates the ContentInfo structure.
%
%  The format of the AcquireContentInfo method is:
%
%      ContentInfo *AcquireContentInfo(void)
%
*/
WizardExport ContentInfo *AcquireContentInfo(void)
{
  ContentInfo
    *content_info;

  size_t
    version;

  content_info=(ContentInfo *) AcquireWizardMemory(sizeof(*content_info));
  if (content_info == (ContentInfo *) NULL)
    ThrowWizardFatalError(CipherDomain,MemoryError);
  (void) ResetWizardMemory(content_info,0,sizeof(*content_info));
  content_info->cipher=AESCipher;
  content_info->mode=CTRMode;
  content_info->authenticate_method=SecretAuthenticateMethod;
  content_info->key_hash=SHA2256Hash;
  content_info->key_length=512;
  content_info->entropy=BZIPEntropy;
  content_info->level=6;
  content_info->hmac=SHA2256Hash;
  content_info->random_hash=SHA2256Hash;
  content_info->chunksize=262144;
  content_info->timestamp=time((time_t *) NULL);
  content_info->version=ConstantString(GetWizardVersion(&version));
  content_info->protocol_major=CipherProtocolMajor;
  content_info->protocol_minor=CipherProtocolMinor;
  content_info->signature=WizardSignature;
  return(content_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C o n t e n t I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyContentInfo() zeros memory associated with the ContentInfo
%  structure.
%
%  The format of the DestroyContentInfo method is:
%
%      ContentInfo *DestroyContentInfo(ContentInfo *content_info)
%
%  A description of each parameter follows:
%
%    o content_info: The content info.
%
*/
WizardExport ContentInfo *DestroyContentInfo(ContentInfo *content_info)
{
  WizardAssert(CipherDomain,content_info != (ContentInfo *) NULL);
  WizardAssert(CipherDomain,content_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
    content_info->content);
  if (content_info->random_info != (RandomInfo *) NULL)
    content_info->random_info=DestroyRandomInfo(content_info->random_info);
  if (content_info->hmac_info != (HMACInfo *) NULL)
    content_info->hmac_info=DestroyHMACInfo(content_info->hmac_info);
  if (content_info->entropy_info != (EntropyInfo *) NULL)
    content_info->entropy_info=DestroyEntropyInfo(content_info->entropy_info);
  if (content_info->passphrase != (char *) NULL)
    content_info->passphrase=DestroyString(content_info->passphrase);
  if (content_info->id != (char *) NULL)
    content_info->id=DestroyString(content_info->id);
  if (content_info->keyring != (char *) NULL)
    content_info->keyring=DestroyString(content_info->keyring);
  if (content_info->authenticate_info != (AuthenticateInfo *) NULL)
    content_info->authenticate_info=DestroyAuthenticateInfo(
      content_info->authenticate_info);
  if (content_info->nonce != (char *) NULL)
    content_info->nonce=DestroyString(content_info->nonce);
  if (content_info->cipher_info != (CipherInfo *) NULL)
    content_info->cipher_info=DestroyCipherInfo(content_info->cipher_info);
  if (content_info->properties != (char *) NULL)
    content_info->properties=DestroyString(content_info->properties);
  if (content_info->content != (char *) NULL)
    content_info->content=DestroyString(content_info->content);
  if (content_info->version != (char *) NULL)
    content_info->version=DestroyString(content_info->version);
  if (content_info->plainblob != (BlobInfo *) NULL)
    content_info->plainblob=DestroyBlob(content_info->plainblob);
  if (content_info->cipherblob != (BlobInfo *) NULL)
    content_info->cipherblob=DestroyBlob(content_info->cipherblob);
  content_info->signature=(~WizardSignature);
  content_info=(ContentInfo *) RelinquishWizardMemory(content_info);
  return(content_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n t e n t I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetContentInfo() initializes the content info structure from the specified
%  blob.
%
%  The format of the GetContentInfo method is:
%
%      WizardBooleanType GetContentInfo(ContentInfo *content_info,
%        BLobInfo *cipher_blob,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o content_info: the content info.
%
%    o cipher_blob: the cipher blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType GetContentInfo(ContentInfo *content_info,
  BlobInfo *cipher_blob,ExceptionInfo *exception)
{
  char
    *digest,
    key[WizardPathExtent],
    *options,
    *target_digest;

  const char
    *content,
    *tag;

  HashInfo
    *hash_info;

  int
    c;

  ssize_t
    type;

  ssize_t
    count;

  size_t
    bytes,
    length;

  StringInfo
    *properties;

  XMLTreeInfo
    *child,
    *node,
    *rdf;

  WizardAssert(CipherDomain,content_info != (ContentInfo *) NULL);
  WizardAssert(CipherDomain,content_info->signature == WizardSignature);
  WizardAssert(CipherDomain,cipher_blob != (BlobInfo *) NULL);
  WizardAssert(CipherDomain,exception != (ExceptionInfo *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
    GetBlobFilename(cipher_blob));
  (void) ResetWizardMemory(key,0,sizeof(key));
  bytes=0;
  digest=(char *) NULL;
  for (c=ReadBlobByte(cipher_blob); (c != '>') && (c != EOF); )
  {
    length=WizardPathExtent;
    options=AcquireString((char *) NULL);
    while ((isgraph(c) != WizardFalse) && (c != '>') && (c != EOF))
    {
       char
        *p;

      if (isalnum(c) == WizardFalse)
        c=ReadBlobByte(cipher_blob);
      else
        {
          /*
            Get the key.
          */
          p=key;
          do
          {
            if (isspace((int) ((unsigned char) c)) != 0)
              break;
            if (c == (int) '=')
              break;
            if ((size_t) (p-key) < WizardPathExtent)
              *p++=(char) c;
            c=ReadBlobByte(cipher_blob);
          } while (c != EOF);
          *p='\0';
          p=options;
          while (isspace((int) ((unsigned char) c)) != 0)
            c=ReadBlobByte(cipher_blob);
          if (c == (int) '=')
            {
              /*
                Get the value.
              */
              c=ReadBlobByte(cipher_blob);
              while ((c != (int) '?') && (c != EOF))
              {
                if ((size_t) (p-options+1) >= length)
                  {
                    *p='\0';
                    length<<=1;
                    options=(char *) ResizeQuantumMemory(options,length+
                      WizardPathExtent,sizeof(*options));
                    if (options == (char *) NULL)
                      break;
                    p=options+strlen(options);
                  }
                if (options == (char *) NULL)
                  {
                    (void) ThrowWizardException(exception,GetWizardModule(),
                      ResourceError,"memory allocation failed: `%s'",
                      GetBlobFilename(cipher_blob));
                    return(WizardFalse);
                  }
                *p++=(char) c;
                c=ReadBlobByte(cipher_blob);
                if (*options != '?')
                  if (isspace((int) ((unsigned char) c)) != 0)
                    break;
              }
            }
          *p='\0';
          /*
            Interpret key.
          */
          switch (*key)
          {
            case 'b':
            case 'B':
            {
              if (LocaleCompare(key,"bytes") == 0)
                {
                  StripString(options);
                  bytes=(size_t) StringToLong(options);
                  break;
                }
              break;
            }
            case 'd':
            case 'D':
            {
              if (LocaleCompare(key,"digest") == 0)
                {
                  StripString(options);
                  digest=AcquireString(options);
                  break;
                }
              break;
            }
            default:
              break;
          }
        }
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(cipher_blob);
    }
    options=DestroyString(options);
  }
  if ((bytes == 0) || (digest == (char *) NULL))
    {
      (void) ThrowWizardException(exception,GetWizardModule(),AuthenticateError,
        "authentication error: `%s'",GetBlobFilename(cipher_blob));
      return(WizardFalse);
    }
  /*
    Verify digest signature.
  */
  c=ReadBlobByte(cipher_blob);
  properties=AcquireStringInfo(bytes);
  count=ReadBlob(cipher_blob,bytes,GetStringInfoDatum(properties));
  if (count != (ssize_t) bytes)
    ThrowFileException(exception,FileError,GetBlobFilename(cipher_blob));
  GetStringInfoDatum(properties)[bytes]='\0';
  hash_info=AcquireHashInfo(SHA2256Hash);
  InitializeHash(hash_info);
  UpdateHash(hash_info,properties);
  FinalizeHash(hash_info);
  target_digest=GetHashHexDigest(hash_info);
  if (strcmp(target_digest,digest) != 0)
    {
      target_digest=DestroyString(target_digest);
      digest=DestroyString(digest);
      hash_info=DestroyHashInfo(hash_info);
      properties=DestroyStringInfo(properties);
      (void) ThrowWizardException(exception,GetWizardModule(),AuthenticateError,
        "authentication error: `%s'",GetBlobFilename(cipher_blob));
      return(WizardFalse);
    }
  hash_info=DestroyHashInfo(hash_info);
  target_digest=DestroyString(target_digest);
  digest=DestroyString(digest);
  /*
    Parse content XML.
  */
  rdf=NewXMLTree((const char *) GetStringInfoDatum(properties),exception);
  properties=DestroyStringInfo(properties);
  if (rdf == (XMLTreeInfo *) NULL)
    return(WizardFalse);
  child=GetXMLTreeChild(rdf,"cipher:Content");
  if (child == (XMLTreeInfo *) NULL)
    child=GetXMLTreeChild(rdf,"rdf:Description");
  if (child == (XMLTreeInfo *) NULL)
    {
      rdf=DestroyXMLTree(rdf);
      (void) ThrowWizardException(exception,GetWizardModule(),AuthenticateError,
        "authentication error: `%s'",GetBlobFilename(cipher_blob));
      return(WizardFalse);
    }
  node=GetXMLTreeChild(child,(const char *) NULL);
  while (node != (XMLTreeInfo *) NULL)
  {
    tag=GetXMLTreeTag(node);
    content=GetXMLTreeContent(node);
    if (strncmp(tag,"cipher:",7) != 0)
      continue;
    tag+=7;
    switch (*tag)
    {
      case 'a':
      {
        if (strcmp(tag,"authenticate") == 0)
          {
            ssize_t
              method;

            method=ParseWizardOption(WizardAuthenticateOptions,WizardFalse,
              content);
            if (method < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized authentication method: `%s'",content);
            content_info->authenticate_method=(AuthenticateMethod) method;
            break;
          }
        break;
      }
      case 'c':
      {
        if (strcmp(tag,"chunksize") == 0)
          {
            content_info->chunksize=(unsigned int) StringToUnsignedLong(content);
            break;
          }
        if (strcmp(tag,"create-date") == 0)
          {
            if (*ParseWizardTime(content,&content_info->create_date) != '\0')
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized timestamp: `%s'",content);
            break;
          }
        break;
      }
      case 'e':
      {
        if (strcmp(tag,"entropy") == 0)
          {
            type=ParseWizardOption(WizardEntropyOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized entropy type: `%s'",content);
            content_info->entropy=(EntropyType) type;
            break;
          }
        break;
      }
      case 'h':
      {
        if (strcmp(tag,"hmac") == 0)
          {
            type=ParseWizardOption(WizardHashOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized HMAC hash: `%s'",content);
            content_info->hmac=(HashType) type;
            break;
          }
        break;
      }
      case 'i':
      {
        if (strcmp(tag,"id") == 0)
          {
            content_info->id=ConstantString(content);
            break;
          }
        break;
      }
      case 'k':
      {
        if (strcmp(tag,"key-hash") == 0)
          {
            type=ParseWizardOption(WizardHashOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized key hash: `%s'",content);
            content_info->key_hash=(HashType) type;
            break;
          }
        if (strcmp(tag,"key-length") == 0)
          {
            content_info->key_length=(unsigned int) StringToDouble(
              content,(char **) NULL);
            break;
          }
        break;
      }
      case 'l':
      {
        if (strcmp(tag,"level") == 0)
          {
            type=ParseWizardOption(WizardEntropyLevelOptions,WizardFalse,
              content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized entropy level: `%s'",content);
            content_info->level=(unsigned int) type;
            break;
          }
        break;
      }
      case 'm':
      {
        if (strcmp(tag,"mode") == 0)
          {
            type=ParseWizardOption(WizardModeOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized cipher mode: `%s'",content);
            content_info->mode=(CipherMode) type;
            break;
          }
        if (strcmp(tag,"modify-date") == 0)
          {
            if (*ParseWizardTime(content,&content_info->modify_date) != '\0')
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized timestamp: `%s'",content);
            break;
          }
        break;
      }
      case 'n':
      {
        if (strcmp(tag,"nonce") == 0)
          {
            content_info->nonce=ConstantString(content);
            break;
          }
        break;
      }
      case 'r':
      {
        if (strcmp(tag,"random-hash") == 0)
          {
            type=ParseWizardOption(WizardHashOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized random hash: `%s'",content);
            content_info->random_hash=(HashType) type;
            break;
          }
        break;
      }
      case 's':
      {
        if (strcmp(tag,"session") == 0)  /* deprecated */
          {
            content_info->id=ConstantString(content);
            break;
          }
        break;
      }
      case 't':
      {
        if (strcmp(tag,"timestamp") == 0)
          {
            if (*ParseWizardTime(content,&content_info->timestamp) != '\0')
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized timestamp: `%s'",content);
            break;
          }
        if (strcmp(tag,"type") == 0)
          {
            type=ParseWizardOption(WizardCipherOptions,WizardFalse,content);
            if (type < 0)
              (void) ThrowWizardException(exception,GetWizardModule(),
                OptionError,"unrecognized cipher type: `%s'",content);
            content_info->cipher=(CipherType) type;
            break;
          }
        break;
      }
      case 'v':
      {
        if (strcmp(tag,"version") == 0)
          {
            if (content_info->version != (char *) NULL)
              content_info->version=DestroyString(content_info->version);
            content_info->version=ConstantString(content);
            break;
          }
        break;
      }
      default:
        break;
    }
    node=GetXMLTreeSibling(node);
  }
  rdf=DestroyXMLTree(rdf);
  /*
    Consume trailer (i.e. <?cipherpacket?>).
  */
  while ((c != EOF) && (c != '>'))
  {
    c=ReadBlobByte(cipher_blob);
    while (isspace((int) ((unsigned char) c)) != 0)
      c=ReadBlobByte(cipher_blob);
  }
  c=ReadBlobByte(cipher_blob);
  c=ReadBlobByte(cipher_blob);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t C i p h e r P r o p e r t i e s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintCipherProperties() prints properties associated with a cipher packet
%  to the specified file.
%
%  The format of the PrintCipherProperties method is:
%
%      WizardBooleanType PrintCipherProperties(const CipherInfo *content_info,
%        FILE *file)
%
%  A description of each parameter follows:
%
%    o content_info: The content info.
%
%    o file: the file, typically stdout.
%
*/
WizardExport WizardBooleanType PrintCipherProperties(
  const ContentInfo *content_info,FILE *file)
{
  char
    chunksize[WizardPathExtent],
    timestamp[WizardPathExtent];

  (void) fprintf(file,"Filename: %s\n",content_info->content);
  if (content_info->properties != (char *) NULL)
    (void) fprintf(file,"  Properties: %s\n",content_info->properties);
  (void) FormatWizardTime(content_info->modify_date,WizardPathExtent,timestamp);
  (void) fprintf(file,"  Cipher:\n");
  (void) fprintf(file,"    type: %s\n",WizardOptionToMnemonic(
    WizardCipherOptions,content_info->cipher));
  (void) fprintf(file,"    mode: %s\n",WizardOptionToMnemonic(WizardModeOptions,
    content_info->mode));
  (void) fprintf(file,"    nonce: %s\n",content_info->nonce);
  (void) fprintf(file,"  Authenticate:\n");
  (void) fprintf(file,"    method: %s\n",WizardOptionToMnemonic(
    WizardAuthenticateOptions,content_info->authenticate_method));
  (void) fprintf(file,"  Key:\n");
  (void) fprintf(file,"    hash: %s\n",WizardOptionToMnemonic(WizardHashOptions,
    content_info->key_hash));
  (void) fprintf(file,"    length: %.20g\n",(double) content_info->key_length);
  (void) fprintf(file,"    id: %s\n",content_info->id);
  (void) fprintf(file,"  Entropy Generator:\n");
  (void) fprintf(file,"    type: %s\n",WizardOptionToMnemonic(
    WizardEntropyOptions,content_info->entropy));
  (void) fprintf(file,"    level: %.20g\n",(double) content_info->level);
  (void) fprintf(file,"  Keyed-Hashed Message Authentication Code:\n");
  (void) fprintf(file,"    hash: %s\n",WizardOptionToMnemonic(WizardHashOptions,
    content_info->hmac));
  (void) fprintf(file,"  Random Generator:\n");
  (void) fprintf(file,"    hash: %s\n",WizardOptionToMnemonic(WizardHashOptions,
    content_info->random_hash));
  (void) FormatWizardSize(content_info->chunksize,WizardFalse,WizardPathExtent,
    chunksize);
  (void) fprintf(file,"  Chunksize: %sB\n",chunksize);
  (void) fprintf(file,"  Dates:\n");
  (void) fprintf(file,"    modify: %s\n",timestamp);
  (void) FormatWizardTime(content_info->create_date,WizardPathExtent,timestamp);
  (void) fprintf(file,"    create: %s\n",timestamp);
  (void) FormatWizardTime(content_info->timestamp,WizardPathExtent,timestamp);
  (void) fprintf(file,"    timestamp: %s\n",timestamp);
  (void) fprintf(file,"  Protocol: %u.%u\n",(unsigned int)
    content_info->protocol_major,(unsigned int) content_info->protocol_minor);
  if (content_info->version != (char *) NULL)
    (void) fprintf(file,"  Version: %s\n",content_info->version);
  return(ferror(file) != 0 ? WizardFalse : WizardTrue);
}
