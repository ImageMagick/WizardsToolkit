/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS   TTTTT  RRRR   IIIII  N   N   GGGG                  %
%                  SS        T    R   R    I    NN  N  G                      %
%                   SSS      T    RRRR     I    N N N  G GGG                  %
%                     SS     T    R R      I    N  NN  G   G                  %
%                  SSSSS     T    R  R   IIIII  N   N   GGGG                  %
%                                                                             %
%                                                                             %
%                        Wizard's Toolkit String Methods                      %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.wizards-toolkit.org/script/license.php                        %
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
%
*/

/*
  Inc.,de declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/cipher.h"
#include "wizard/crc64.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/string_.h"

/*
  Structure declarations.
*/
struct _StringInfo
{
  char
    path[MaxTextExtent];

  unsigned char
    *datum;

  size_t
    length;

  time_t
    timestamp;

  unsigned long
    signature;
};

/*
  Static declarations.
*/
#if !defined(WIZARDSTOOLKIT_HAVE_STRCASECMP) || !defined(WIZARDSTOOLKIT_HAVE_STRNCASECMP)
static const unsigned char
  AsciiMap[] =
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xe1, 0xe2, 0xe3, 0xe4, 0xc5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb,
    0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff,
  };
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S t r i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireString() allocates memory for a string and copies the source string
%  to that memory location (and returns it).
%
%  The format of the AcquireString method is:
%
%      char *AcquireString(const char *source)
%
%  A description of each parameter follows:
%
%    o allocated_string:  Method AcquireString returns a copy of the source
%      string.
%
%    o source: A character string.
%
*/
WizardExport char *AcquireString(const char *source)
{
  char
    *destination;

  size_t
    length;

  length=0;
  if (source != (char *) NULL)
    length+=strlen(source);
  destination=(char *) NULL;
  if (~length >= MaxTextExtent)
    destination=(char *) AcquireQuantumMemory(length+MaxTextExtent,
      sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  *destination='\0';
  if (source != (char *) NULL)
    (void) CopyWizardString(destination,source,(length+1UL)*
      sizeof(*destination));
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireStringInfo() allocates the StringInfo structure.
%
%  The format of the AcquireStringInfo method is:
%
%      StringInfo *AcquireStringInfo(const size_t length)
%
%  A description of each parameter follows:
%
%    o length: The string length.
%
*/
WizardExport StringInfo *AcquireStringInfo(const size_t length)
{
  StringInfo
    *string_info;

  string_info=(StringInfo *) AcquireAlignedMemory(1,sizeof(*string_info));
  if (string_info == (StringInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ResetWizardMemory(string_info,0,sizeof(*string_info));
  string_info->timestamp=time((time_t *) NULL);
  string_info->signature=WizardSignature;
  string_info->length=length;
  if (string_info->length != 0)
    {
      string_info->datum=(unsigned char *) NULL;
      if (~string_info->length >= MaxCipherBlocksize)
        string_info->datum=(unsigned char *) AcquireQuantumMemory(
          string_info->length+MaxCipherBlocksize,sizeof(*string_info->datum));
      if (string_info->datum == (unsigned char *) NULL)
        ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
    }
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e S t r i n g                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneString() allocates memory for the destination string and copies
%  the source string to that memory location.
%
%  The format of the CloneString method is:
%
%      char *CloneString(char **destination,const char *source)
%
%  A description of each parameter follows:
%
%    o destination:  A pointer to a character string.
%
%    o source: A character string.
%
*/
WizardExport char *CloneString(char **destination,const char *source)
{
  size_t
    length;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,destination != (char **) NULL);
  if (source == (const char *) NULL)
    {
      if (*destination != (char *) NULL)
        *destination=(char *) RelinquishWizardMemory(*destination);
      return(*destination);
    }
  if (*destination == (char *) NULL)
    {
      *destination=AcquireString(source);
      return(*destination);
    }
  length=strlen(source);
  if (~length < MaxTextExtent)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  *destination=(char *) ResizeQuantumMemory(*destination,length+MaxTextExtent,
    sizeof(*destination));
  if (*destination == (char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) CopyWizardString(*destination,source,(length+1)*sizeof(*destination));
  return(*destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneStringInfo() clones a copy of the StringInfo structure.
%
%  The format of the CloneStringInfo method is:
%
%      StringInfo *CloneStringInfo(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
*/
WizardExport StringInfo *CloneStringInfo(const StringInfo *string_info)
{
  StringInfo
    *clone_info;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  clone_info=AcquireStringInfo(string_info->length);
  if (string_info->length != 0)
    (void) CopyWizardMemory(clone_info->datum,string_info->datum,string_info->length+
      MaxCipherBlocksize);
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareStringInfo() compares the two datums target and source.  It returns
%  an integer less than, equal to, or greater than zero if target is found,
%  respectively, to be less than, to match, or be greater than source.
%
%  The format of the CompareStringInfo method is:
%
%      int CompareStringInfo(const StringInfo *target,const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o target: The target string.
%
%    o source: The source string.
%
*/

static inline size_t WizardMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

WizardExport int CompareStringInfo(const StringInfo *target,
  const StringInfo *source)
{
  int
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,target != (StringInfo *) NULL);
  WizardAssert(StringDomain,target->signature == WizardSignature);
  WizardAssert(StringDomain,source != (StringInfo *) NULL);
  WizardAssert(StringDomain,source->signature == WizardSignature);
  status=memcmp(target->datum,source->datum,WizardMin(target->length,
    source->length));
  if (status != 0)
    return(status);
  if (target->length == source->length)
    return(0);
  return(target->length < source->length ? -1 : 1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e S t r i n g                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateString() appends a copy of string source, including the
%  terminating null character, to the end of string destination.
%
%  The format of the ConcatenateString method is:
%
%      WizardBooleanType ConcatenateString(char **destination,
%        const char *source)
%
%  A description of each parameter follows:
%
%    o destination:  A pointer to a character string.
%
%    o source: A character string.
%
*/
WizardExport WizardBooleanType ConcatenateString(char **destination,
  const char *source)
{
  size_t
    length,
    source_length;

  assert(destination != (char **) NULL);
  if (source == (const char *) NULL)
    return(WizardTrue);
  if (*destination == (char *) NULL)
    {
      *destination=AcquireString(source);
      return(WizardTrue);
    }
  length=strlen(*destination);
  source_length=strlen(source);
  if (~length < source_length)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  length+=source_length;
  if (~length < MaxTextExtent)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  *destination=(char *) ResizeQuantumMemory(*destination,length+MaxTextExtent,
    sizeof(*destination));
  if (*destination == (char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ConcatenateWizardString(*destination,source,
    (length+1)*sizeof(*destination));
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e W i z a r d S t r i n g                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateWizardString() concatenates the source string to the destination
%  string.  The destination buffer is always null-terminated even if the
%  string must be truncated.
%
%  The format of the ConcatenateWizardString method is:
%
%      size_t ConcatenateWizardString(char *destination,const char *source,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o destination: The destination string.
%
%    o source: The source string.
%
%    o length: The length of the destination string.
%
*/
WizardExport size_t ConcatenateWizardString(char *destination,
  const char *source,const size_t length)
{
#if !defined(WIZARDSTOOLKIT_HAVE_STRLCAT)
  register char
    *q;

  register const char
    *p;

  register size_t
    i;

  size_t
    count;

  assert(destination != (char *) NULL);
  assert(source != (const char *) NULL);
  assert(length >= 1);
  p=source;
  q=destination;
  i=length;
  while ((i-- != 0) && (*q != '\0'))
    q++;
  count=(size_t) (q-destination);
  i=length-count;
  if (i == 0)
    return(count+strlen(p));
  while (*p != '\0')
  {
    if (i != 1)
      {
        *q++=(*p);
        i--;
      }
    p++;
  }
  *q='\0';
  return(count+(p-source));
#else
  return(strlcat(destination,source,length));
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n f i g u r e F i l e T o S t r i n g I n f o                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureFileToStringInfo() returns the contents of a configure file as a
%  string.
%
%  The format of the ConfigureFileToStringInfo method is:
%
%      StringInfo *ConfigureFileToStringInfo(const char *filename)
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The filename.
%
*/
WizardExport StringInfo *ConfigureFileToStringInfo(const char *filename)
{
  char
    *string;

  int
    file;

  WizardOffsetType
    offset;

  size_t
    length;

  StringInfo
    *string_info;

  void
    *map;

  WizardAssert(StringDomain,filename != (const char *) NULL);
  file=open(filename,O_RDONLY | O_BINARY);
  if (file == -1)
    return((StringInfo *) NULL);
  offset=(WizardOffsetType) WizardSeek(file,0,SEEK_END);
  if ((offset < 0) || (offset != (WizardOffsetType) ((ssize_t) offset)))
    {
      file=close(file)-1;
      return((StringInfo *) NULL);
    }
  length=(size_t) offset;
  string=(char *) NULL;
  if (~length > MaxCipherBlocksize)
    string=(char *) AcquireQuantumMemory(length+MaxCipherBlocksize,
      sizeof(*string));
  if (string == (char *) NULL)
    {
      file=close(file)-1;
      return((StringInfo *) NULL);
    }
  map=MapBlob(file,ReadMode,0,length);
  if (map != (void *) NULL)
    {
      (void) CopyWizardMemory(string,map,length);
      (void) UnmapBlob(map,length);
    }
  else
    {
      register size_t
        i;

      ssize_t
        count;

      (void) WizardSeek(file,0,SEEK_SET);
      for (i=0; i < length; i+=count)
      {
        count=read(file,string+i,(size_t) WizardMin(length-i,(size_t)
          SSIZE_MAX));
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
      }
      if (i < length)
        {
          file=close(file)-1;
          string=(char *) RelinquishWizardMemory(string);
          return((StringInfo *) NULL);
        }
    }
  string[length]='\0';
  file=close(file)-1;
  string_info=AcquireStringInfo(0);
  (void) CopyWizardString(string_info->path,filename,MaxTextExtent);
  string_info->length=strlen(string)+1;
  string_info->datum=(unsigned char *) string;
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n s t a n t S t r i n g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstantString() allocates memory for a string and copies the source string
%  to that memory location (and returns it).  Use it for strings that you do
%  do not expect to change over its lifetime.
%
%  The format of the ConstantString method is:
%
%      char *ConstantString(const char *source)
%
%  A description of each parameter follows:
%
%    o source: A character string.
%
*/
WizardExport char *ConstantString(const char *source)
{
  char
    *destination;

  size_t
    length;

  length=0;
  if (source != (char *) NULL)
    length+=strlen(source);
  destination=(char *) NULL;
  if (~length >= 1UL)
    destination=(char *) AcquireQuantumMemory(length+1UL,sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  *destination='\0';
  if (source != (char *) NULL)
    (void) CopyWizardString(destination,source,(length+1UL)*
      sizeof(*destination));
  return(destination);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n c a t e n a t e S t r i n g I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConcatenateStringInfo() concatenates the source string to the destination
%  string.
%
%  The format of the ConcatentateStringInfo method is:
%
%      void ConcatentateStringInfo(StringInfo *string_info,
%        const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
%    o source: The source string.
%
*/
WizardExport void ConcatenateStringInfo(StringInfo *string_info,
  const StringInfo *source)
{
  size_t
    length;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  WizardAssert(StringDomain,source != (const StringInfo *) NULL);
  length=string_info->length;
  if (~length < source->length)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  SetStringInfoLength(string_info,string_info->length+source->length);
  (void) CopyWizardMemory(string_info->datum+length,source->datum,
    source->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y W i z a r d S t r i n g                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyWizardString() copies the source string to the destination string.  The
%  destination buffer is always null-terminated even if the string must be
%  truncated.  The return value is the minimum of the  ource string length
%  or the length parameter.
%
%  The format of the CopyWizardString method is:
%
%      size_t CopyWizardString(const char *destination,char *source,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o destination: The destination string.
%
%    o source: The source string.
%
%    o length: The length of the destination string.
%
*/
WizardExport size_t CopyWizardString(char *destination,const char *source,
  const size_t length)
{
  register char
    *q;

  register const char
    *p;

  register size_t
    n;

  p=source;
  q=destination;
  for (n=length; n > 4; n-=4)
  {
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
    *q=(*p++);
    if (*q == '\0')
      return((size_t) (p-source-1));
    q++;
  }
  if (n != 0)
    for (n--; n != 0; n--)
    {
      *q=(*p++);
      if (*q == '\0')
        return((size_t) (p-source-1));
      q++;
    }
  if (length != 0)
    *q='\0';
  return((size_t) (p-source-1));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyString() destorys memory associated with a string.
%
%  The format of the DestroyString method is:
%
%      char *DestroyString(char *string)
%
%  A description of each parameter follows:
%
%    o string: The string.
%
*/
WizardExport char *DestroyString(char *string)
{
  return((char *) RelinquishWizardMemory(string));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyStringInfo() zeros memory associated with the StringInfo structure.
%
%  The format of the DestroyStringInfo method is:
%
%      StringInfo *DestroyStringInfo(StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
*/
WizardExport StringInfo *DestroyStringInfo(StringInfo *string_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  if (string_info->datum != (unsigned char *) NULL)
    string_info->datum=(unsigned char *) RelinquishWizardMemory(
      string_info->datum);
  string_info->signature=(~WizardSignature);
  string_info=(StringInfo *) RelinquishWizardMemory(string_info);
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S t r i n g L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyStringList() zeros memory associated with a string list.
%
%  The format of the DestroyStringList method is:
%
%      char **DestroyStringList(char **list)
%
%  A description of each parameter follows:
%
%    o list: The string list.
%
*/
WizardExport char **DestroyStringList(char **list)
{
  register long
    i;

  assert(list != (char **) NULL);
  for (i=0; list[i] != (char *) NULL; i++)
    list[i]=DestroyString(list[i]);
  list=(char **) RelinquishWizardMemory(list);
  return(list);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o S t r i n g                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToString() returns the contents of a file as a string.
%
%  The format of the FileToString method is:
%
%      char *FileToString(const char *filename,const size_t extent,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The filename.
%
%    o extent: Maximum length of the string.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport char *FileToString(const char *filename,const size_t extent,
  ExceptionInfo *exception)
{
  size_t
    length;

  WizardAssert(StringDomain,filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  WizardAssert(StringDomain,exception != (ExceptionInfo *) NULL);
  return((char *) FileToBlob(filename,extent,&length,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o S t r i n g I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToStringInfo() returns the contents of a file as a string.
%
%  The format of the FileToStringInfo method is:
%
%      StringInfo *FileToStringInfo(const char *filename,const size_t extent,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The filename.
%
%    o extent: Maximum length of the string.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport StringInfo *FileToStringInfo(const char *filename,
  const size_t extent,ExceptionInfo *exception)
{
  StringInfo
    *string_info;

  WizardAssert(StringDomain,filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  WizardAssert(StringDomain,exception != (ExceptionInfo *) NULL);
  string_info=AcquireStringInfo(0);
  (void) CopyWizardString(string_info->path,filename,MaxTextExtent);
  string_info->datum=FileToBlob(filename,extent,&string_info->length,exception);
  if (string_info->datum == (unsigned char *) NULL)
    {
      string_info=DestroyStringInfo(string_info);
      return((StringInfo *) NULL);
    }
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t W i z a r d S i z e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatWizardSize() converts a size to a human readable format, for example,
%  14kb, 234mb, 2.7gb, or 3.0tb.  Scaling is done by repetitively dividing by
%  1000.
%
%  The format of the FormatWizardSize method is:
%
%      long FormatWizardSize(const WizardSizeType size,char *format)
%
%  A description of each parameter follows:
%
%    o size:  convert this size to a human readable format.
%
%    o bi:  use power of two rather than power of ten.
%
%    o format:  human readable format.
%
*/
WizardExport long FormatWizardSize(const WizardSizeType size,
  const WizardBooleanType bi,char *format)
{
  const char
    **units;

  double
    bytes,
    length;

  long
    count;

  register long
    i,
    j;

  static const char
    *bi_units[] =
    {
      "b", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB", (char *) NULL
    },
    *traditional_units[] =
    {
      "b", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", (char *) NULL
    };

  bytes=1000.0;
  units=traditional_units;
  if (bi != WizardFalse)
    {
      bytes=1024.0;
      units=bi_units;
    }
#if defined(_MSC_VER) && (_MSC_VER == 1200)
  length=(double) ((WizardOffsetType) size);
#else
  length=(double) size;
#endif
  for (i=0; (length >= bytes) && (units[i+1] != (const char *) NULL); i++)
    length/=bytes;
  for (j=2; j < 12; j++)
  {
    count=FormatWizardString(format,MaxTextExtent,"%.*g%s",(int) (i+j),length,
      units[i]);
    if (strchr(format,'+') == (char *) NULL)
      break;
  }
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t W i z a r d S t r i n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatWizardString() prints formatted output of a variable argument list.
%
%  The format of the FormatWizardString method is:
%
%      long FormatWizardString(char *string,const size_t length,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o string:  FormatWizardString() returns the formatted string in this
%     character buffer.
%
%   o length: The maximum length of the string.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/

WizardExport long FormatWizardStringList(char *string,const size_t length,
  const char *format,va_list operands)
{
  int
    n;

#if defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF)
  n=vsnprintf(string,length,format,operands);
#else
  n=vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[length-1]='\0';
  return(n);
}

WizardExport long FormatWizardString(char *string,const size_t length,
  const char *format,...)
{
  long
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatWizardStringList(string,length,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  F o r m a t W i z a r d T i m e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatWizardTime() returns the specified time in the Internet date/time
%  format and the length of the timestamp.
%
%  The format of the FormatWizardTime method is:
%
%      long FormatWizardTime(const time_t time,const size_t length,
%        char *timestamp)
%
%  A description of each parameter follows.
%
%   o time:  the time since the Epoch (00:00:00 UTC, January 1, 1970),
%     measured in seconds.
%
%   o length: The maximum length of the string.
%
%   o timestamp:  Return the Internet date/time here.
%
*/
WizardExport long FormatWizardTime(const time_t time,const size_t length,
  char *timestamp)
{
  long
    count;

  struct tm
    gm_time,
    local_time;

  time_t
    timezone;

  assert(timestamp != (char *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_LOCALTIME_R)
  (void) localtime_r(&time,&local_time);
#else
  {
    struct tm
      *my_time;

    my_time=localtime(&time);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(&local_time,my_time,sizeof(local_time));
  }
#endif
#if defined(WIZARDSTOOLKIT_HAVE_GMTIME_R)
  (void) gmtime_r(&time,&gm_time);
#else
  {
    struct tm
      *my_time;

    my_time=gmtime(&time);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(&gm_time,my_time,sizeof(gm_time));
  }
#endif
  timezone=(time_t) ((local_time.tm_min-gm_time.tm_min)/60+
    local_time.tm_hour-gm_time.tm_hour+24*((local_time.tm_year-
    gm_time.tm_year) != 0 ? (local_time.tm_year-gm_time.tm_year) :
    (local_time.tm_yday-gm_time.tm_yday)));
  count=FormatWizardString(timestamp,length,
    "%04d-%02d-%02dT%02d:%02d:%02d%+03ld:00",local_time.tm_year+1900,
    local_time.tm_mon+1,local_time.tm_mday,local_time.tm_hour,
    local_time.tm_min,local_time.tm_sec,(long) timezone);
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E n v i r o n m e n t V a l u e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetEnvironmentValue() returns the environment string that matches the
%  specified name.
%
%  The format of the GetEnvironmentValue method is:
%
%      char *GetEnvironmentValue(const char *name)
%
%  A description of each parameter follows:
%
%    o name: the environment name.
%
*/
WizardExport char *GetEnvironmentValue(const char *name)
{
  const char
    *environment;

  environment=getenv(name);
  if (environment == (const char *) NULL)
    return(0);
  return(ConstantString(environment));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o C R C                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoCRC() returns the CRC-64 of a string.
%
%  The format of the GetStringInfo method is:
%
%      WizardSizeType GetStringInfo(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
*/
WizardExport WizardSizeType GetStringInfoCRC(const StringInfo *string_info)
{
  CRC64Info
    *crc_info;

  WizardSizeType
    crc;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  crc_info=AcquireCRC64Info();
  InitializeCRC64(crc_info);
  UpdateCRC64(crc_info,string_info);
  FinalizeCRC64(crc_info);
  crc=GetCRC64CyclicRedundancyCheck(crc_info);
  crc_info=DestroyCRC64Info(crc_info);
  return(crc);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o D a t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoDatum() returns the datum associated with the string.
%
%  The format of the GetStringInfoDatum method is:
%
%      unsigned char *GetStringInfoDatum(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
WizardExport unsigned char *GetStringInfoDatum(const StringInfo *string_info)
{
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  return(string_info->datum);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o L e n g t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoLength() returns the string length.
%
%  The format of the GetStringInfoLength method is:
%
%      size_t GetStringInfoLength(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
WizardExport size_t GetStringInfoLength(const StringInfo *string_info)
{
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  return(string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S t r i n g I n f o P a t h                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetStringInfoPath() returns the path associated with the string.
%
%  The format of the GetStringInfoPath method is:
%
%      const char *GetStringInfoPath(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: the string info.
%
*/
WizardExport const char *GetStringInfoPath(const StringInfo *string_info)
{
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  return(string_info->path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H e x S t r i n g T o S t r i n g I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HexStringToStringInfo() returns the contents of a file as a string.
%
%  The format of the HexStringToStringInfo method is:
%
%      StringInfo *HexStringToStringInfo(const char *string)
%
%  A description of each parameter follows:
%
%    o string:  The string.
%
*/
WizardExport StringInfo *HexStringToStringInfo(const char *string)
{
  register const unsigned char
    *p;

  register long
    i;

  register unsigned char
    *q;

  unsigned char
    hex_digits[256];

  StringInfo
    *string_info;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string != (const char *) NULL);
  string_info=AcquireStringInfo(strlen(string)/2);
  (void) ResetWizardMemory(hex_digits,0,sizeof(hex_digits));
  hex_digits[(int) '0']=0;
  hex_digits[(int) '1']=1;
  hex_digits[(int) '2']=2;
  hex_digits[(int) '3']=3;
  hex_digits[(int) '4']=4;
  hex_digits[(int) '5']=5;
  hex_digits[(int) '6']=6;
  hex_digits[(int) '7']=7;
  hex_digits[(int) '8']=8;
  hex_digits[(int) '9']=9;
  hex_digits[(int) 'a']=10;
  hex_digits[(int) 'b']=11;
  hex_digits[(int) 'c']=12;
  hex_digits[(int) 'd']=13;
  hex_digits[(int) 'e']=14;
  hex_digits[(int) 'f']=15;
  hex_digits[(int) 'A']=10;
  hex_digits[(int) 'B']=11;
  hex_digits[(int) 'C']=12;
  hex_digits[(int) 'D']=13;
  hex_digits[(int) 'E']=14;
  hex_digits[(int) 'F']=15;
  p=(unsigned char *) string;
  q=string_info->datum;
  for (i=0; i < (long) string_info->length; i++)
  {
    *q=hex_digits[*p++] << 4;
    *q|=hex_digits[*p++];
    q++;
  }
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e C o m p a r e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleCompare() performs a case-insensitive comparison of two strings
%  byte-by-byte, according to the ordering of the current locale encoding.
%  LocaleCompare returns an integer greater than, equal to, or less than 0,
%  if the string pointed to by p is greater than, equal to, or less than the
%  string pointed to by q respectively.  The sign of a non-zero return value
%  is determined by the sign of the difference between the values of the first< %  pair of bytes that differ in the strings being compared.
%
%  The format of the LocaleCompare method is:
%
%      int LocaleCompare(const char *p,const char *q)
%
%  A description of each parameter follows:
%
%    o p: A pointer to a character string.
%
%    o q: A pointer to a character string to compare to p.
%
*/
WizardExport int LocaleCompare(const char *p,const char *q)
{
  if ((p == (char *) NULL) && (q == (char *) NULL))
    return(0);
  if (p == (char *) NULL)
    return(-1);
  if (q == (char *) NULL)
    return(1);
#if defined(WIZARDSTOOLKIT_HAVE_STRCASECMP)
  return(strcasecmp(p,q));
#else
  {
    register int
      c,
      d;

    for ( ; ; )
    {
      c=(int) *((unsigned char *) p);
      d=(int) *((unsigned char *) q);
      if ((c == 0) || (AsciiMap[c] != AsciiMap[d]))
        break;
      p++;
      q++;
    }
    return(AsciiMap[c]-(int) AsciiMap[d]);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e L o w e r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleLower() transforms all of the characters in the supplied
%  null-terminated string, changing all uppercase letters to lowercase.
%
%  The format of the LocaleLower method is:
%
%      void LocaleLower(char *string)
%
%  A description of each parameter follows:
%
%    o string: A pointer to the string to convert to lower-case Locale.
%
*/
WizardExport void LocaleLower(char *string)
{
  register char
    *q;

  assert(string != (char *) NULL);
  for (q=string; *q != '\0'; q++)
    *q=(char) tolower((int) *q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c a l e N C o m p a r e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleNCompare() performs a case-insensitive comparison of two
%  strings byte-by-byte, according to the ordering of the current locale
%  encoding. LocaleNCompare returns an integer greater than, equal to, or
%  less than 0, if the string pointed to by p is greater than, equal to, or
%  less than the string pointed to by q respectively.  The sign of a non-zero
%  return value is determined by the sign of the difference between the
%  values of the first pair of bytes that differ in the strings being
%  compared.  The LocaleNCompare method makes the same comparison as
%  LocaleCompare but looks at a maximum of n bytes.  Bytes following a
%  null byte are not compared.
%
%  The format of the LocaleNCompare method is:
%
%      int LocaleNCompare(const char *p,const char *q,const size_t n)
%
%  A description of each parameter follows:
%
%    o p: A pointer to a character string.
%
%    o q: A pointer to a character string to compare to p.
%
%    o length: The number of characters to compare in strings p & q.
%
*/
WizardExport int LocaleNCompare(const char *p,const char *q,const size_t length)
{
  if (p == (char *) NULL)
    return(-1);
  if (q == (char *) NULL)
    return(1);
#if defined(WIZARDSTOOLKIT_HAVE_STRNCASECMP)
  return(strncasecmp(p,q,length));
#else
  {
    register int
      c,
      d;

    register size_t
      i;

    for (i=length; i != 0; i--)
    {
      c=(int) *((unsigned char *) p);
      d=(int) *((unsigned char *) q);
      if (AsciiMap[c] != AsciiMap[d])
        return(AsciiMap[c]-(int) AsciiMap[d]);
      if (c == 0)
        return(0);
      p++;
      q++;
    }
    return(0);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t W i z a r d S t r i n g                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintWizardString() prints the string to the specified file.
%
%  The format of the PrintWizardString method is:
%
%      long PrintWizardString(FILE *,const char *format,...)
%
%  A description of each parameter follows:
%
%    o file: print to this file.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/
WizardExport long PrintWizardString(FILE *file,const char *format,...)
{
  char
    string[MaxTextExtent];

  long
    length;

  va_list
    operands;

  va_start(operands,format);
  length=FormatWizardStringList(string,MaxTextExtent,format,operands);
  va_end(operands);
  if (length < 0)
    return(-1);
  return((long) fwrite(string,(size_t) length,1,file));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintStringInfo() prints the string.
%
%  The format of the PrintStringInfo method is:
%
%      void PrintStringInfo(FILE *file,const char *id,
%        const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o id: The string id.
%
%    o string_info: The string info.
%
*/
WizardExport void PrintStringInfo(FILE *file,const char *id,
  const StringInfo *string_info)
{
  register unsigned char
    *p,
    *q;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  q=string_info->datum+string_info->length;
  for (p=string_info->datum; p < q; p++)
  {
    if (((int) *p < 32) && (isspace((int) ((unsigned char) *p)) == 0))
      break;
    if (isascii((int) ((unsigned char) *p)) == 0)
      break;
  }
  (void) PrintWizardString(file,"%s(%lu): ",id,(unsigned long)
    string_info->length);
  if (p == q)
    for (p=string_info->datum; p < q; p++)
      (void) PrintWizardString(file,"%c",(int) *p);
  else
    for (p=string_info->datum; p < q; p++)
      (void) PrintWizardString(file,"%02lx",(unsigned long) *p);
  (void) PrintWizardString(file,"\n");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetStringInfo() reset the string to all null bytes.
%
%  The format of the ResetStringInfo method is:
%
%      void ResetStringInfo(StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
*/
WizardExport void ResetStringInfo(StringInfo *string_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  (void) ResetWizardMemory(string_info->datum,0,string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfo() copies the source string to the destination string.
%
%  The format of the SetStringInfo method is:
%
%      void SetStringInfo(StringInfo *string_info,const StringInfo *source)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
%    o source: The source string.
%
*/
WizardExport void SetStringInfo(StringInfo *string_info,
  const StringInfo *source)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  WizardAssert(StringDomain,source != (StringInfo *) NULL);
  WizardAssert(StringDomain,source->signature == WizardSignature);
  if (string_info->length == 0)
    return;
  (void) ResetWizardMemory(string_info->datum,0,string_info->length);
  (void) CopyWizardMemory(string_info->datum,source->datum,
    WizardMin(string_info->length,source->length));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o D a t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoDatum() copies bytes from the source string for the length of
%  the destination string.
%
%  The format of the SetStringInfoDatum method is:
%
%      void SetStringInfoDatum(StringInfo *string_info,
%        const unsigned char *source)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
%    o source: The source string.
%
*/
WizardExport void SetStringInfoDatum(StringInfo *string_info,
  const unsigned char *source)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  if (string_info->length != 0)
    (void) CopyWizardMemory(string_info->datum,source,string_info->length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o L e n g t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoLength() set the string length to the specified value.
%
%  The format of the SetStringInfoLength method is:
%
%      void SetStringInfoLength(StringInfo *string_info,const size_t length)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
%    o length: The string length.
%
*/
WizardExport void SetStringInfoLength(StringInfo *string_info,
  const size_t length)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  string_info->length=length;
  if (~length < MaxCipherBlocksize)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  if (string_info->datum == (unsigned char *) NULL)
    string_info->datum=(unsigned char *) AcquireQuantumMemory(length+
      MaxCipherBlocksize,sizeof(*string_info->datum));
  else
    string_info->datum=(unsigned char *) ResizeQuantumMemory(string_info->datum,
      length+MaxCipherBlocksize,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S t r i n g I n f o D a t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetStringInfoPath() sets the path associated with the string.
%
%  The format of the SetStringInfoPath method is:
%
%      void SetStringInfoPath(StringInfo *string_info,const char *path)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
%    o path: The path.
%
*/
WizardExport void SetStringInfoPath(StringInfo *string_info,const char *path)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  WizardAssert(StringDomain,path != (const char *) NULL);
  (void) CopyWizardString(string_info->path,path,MaxTextExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p l i t S t r i n g I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SplitStringInfo() splits a string into two and returns it.
%
%  The format of the SplitStringInfo method is:
%
%      StringInfo *SplitStringInfo(StringInfo *string_info,const size_t offset)
%
%  A description of each parameter follows:
%
%    o string_info: The string info.
%
*/
WizardExport StringInfo *SplitStringInfo(StringInfo *string_info,
  const size_t offset)
{
  StringInfo
    *split_info;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string_info != (StringInfo *) NULL);
  WizardAssert(StringDomain,string_info->signature == WizardSignature);
  if (offset > string_info->length)
    return((StringInfo *) NULL);
  split_info=AcquireStringInfo(offset);
  SetStringInfo(split_info,string_info);
  (void) CopyWizardMemory(string_info->datum,string_info->datum+offset,
    string_info->length-offset+MaxCipherBlocksize);
  SetStringInfoLength(string_info,string_info->length-offset);
  return(split_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g I n f o T o H e x S t r i n g                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringInfoToHexString() converts a string info string to a C string.
%
%  The format of the StringInfoToHexString method is:
%
%      char *StringInfoToHexString(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string.
%
*/
WizardExport char *StringInfoToHexString(const StringInfo *string_info)
{
  char
    *string;

  register const unsigned char
    *p;

  register long
    i;

  register unsigned char
    *q;

  size_t
    length;

  unsigned char
    hex_digits[16];

  length=string_info->length;
  if (~length < MaxTextExtent)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  string=(char *) AcquireQuantumMemory(length+MaxTextExtent,2*sizeof(*string));
  if (string == (char *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  hex_digits[0]='0';
  hex_digits[1]='1';
  hex_digits[2]='2';
  hex_digits[3]='3';
  hex_digits[4]='4';
  hex_digits[5]='5';
  hex_digits[6]='6';
  hex_digits[7]='7';
  hex_digits[8]='8';
  hex_digits[9]='9';
  hex_digits[10]='a';
  hex_digits[11]='b';
  hex_digits[12]='c';
  hex_digits[13]='d';
  hex_digits[14]='e';
  hex_digits[15]='f';
  p=string_info->datum;
  q=(unsigned char *) string;
  for (i=0; i < (long) string_info->length; i++)
  {
    *q++=hex_digits[(*p >> 4) & 0x0f]; 
    *q++=hex_digits[*p & 0x0f]; 
    p++;
  }
  *q='\0';
  return(string);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g I n f o T o S t r i n g                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringInfoToString() converts a string info string to a C string.
%
%  The format of the StringInfoToString method is:
%
%      char *StringInfoToString(const StringInfo *string_info)
%
%  A description of each parameter follows:
%
%    o string_info: The string.
%
*/
WizardExport char *StringInfoToString(const StringInfo *string_info)
{
  char
    *string;

  size_t
    length;

  string=(char *) NULL;
  length=string_info->length;
  if (~length < MaxTextExtent)
    return((char *) NULL);
  string=(char *) AcquireQuantumMemory(length+MaxTextExtent,sizeof(*string));
  if (string == (char *) NULL)
    return((char *) NULL);
  (void) CopyWizardString(string,(char *) string_info->datum,(length+1)*
    sizeof(*string));
  return(string);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S t r i n g T o A r g v                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToArgv() converts a text string into command line arguments.
%
%  The format of the StringToArgv method is:
%
%      char **StringToArgv(const char *text,int *argc)
%
%  A description of each parameter follows:
%
%    o argv:  Method StringToArgv returns the string list unless an error
%      occurs, otherwise NULL.
%
%    o text:  Specifies the string to segment into a list.
%
%    o argc:  This integer pointer returns the number of arguments in the
%      list.
%
*/
WizardExport char **StringToArgv(const char *text,int *argc)
{
  char
    **argv;

  register char
    *p,
    *q;

  register long
    i;

  *argc=0;
  if (text == (char *) NULL)
    return((char **) NULL);
  /*
    Determine the number of arguments.
  */
  for (p=(char *) text; *p != '\0'; )
  {
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    (*argc)++;
    if (*p == '"')
      for (p++; (*p != '"') && (*p != '\0'); p++) ;
    if (*p == '\'')
      for (p++; (*p != '\'') && (*p != '\0'); p++) ;
    while ((isspace((int) ((unsigned char) *p)) == 0) && (*p != '\0'))
      p++;
  }
  argv=(char **) AcquireQuantumMemory((size_t) (*argc+1),sizeof(*argv));
  if (argv == (char **) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  /*
    Convert string to an ASCII list.
  */
  p=(char *) text;
  for (i=0; i < (long) *argc; i++)
  {
    while (isspace((int) ((unsigned char) *p)) != 0)
      p++;
    q=p;
    if (*q == '"')
      {
        p++;
        for (q++; (*q != '"') && (*q != '\0'); q++) ;
      }
    else
      if (*q == '\'')
        {
          for (q++; (*q != '\'') && (*q != '\0'); q++) ;
          q++;
        }
      else
        while ((isspace((int) ((unsigned char) *q)) == 0) && (*q != '\0'))
          q++;
    argv[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+MaxTextExtent,
      sizeof(**argv));
    if (argv[i] == (char *) NULL)
      {
        for (i--; i >= 0; i--)
          argv[i]=(char *) RelinquishWizardMemory(argv[i]);
        argv=(char **) RelinquishWizardMemory(argv);
        ThrowFatalException(StringFatalError,"memory allocation failed `%s'");
      }
    (void) CopyWizardString(argv[i],p,(size_t) (q-p+1));
    p=q;
    while ((isspace((int) ((unsigned char) *p)) == 0) && (*p != '\0'))
      p++;
  }
  argv[i]=(char *) NULL;
  return(argv);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i n g T o S t r i n g I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StringToStringInfo() returns the contents of a file as a string.
%
%  The format of the StringToStringInfo method is:
%
%      StringInfo *StringToStringInfo(const char *string)
%
%  A description of each parameter follows:
%
%    o string:  The string.
%
*/
WizardExport StringInfo *StringToStringInfo(const char *string)
{
  StringInfo
    *string_info;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(StringDomain,string != (const char *) NULL);
  string_info=AcquireStringInfo(strlen(string));
  SetStringInfoDatum(string_info,(const unsigned char *) string);
  return(string_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i p S t r i n g                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StripString() strips any whitespace or quotes from the beginning and end of
%  a string of characters.
%
%  The format of the StripString method is:
%
%      void StripString(char *message)
%
%  A description of each parameter follows:
%
%    o message: Specifies an array of characters.
%
*/
WizardExport void StripString(char *message)
{
  register char
    *p,
    *q;

  size_t
    length;

  assert(message != (char *) NULL);
  if (*message == '\0')
    return;
  length=strlen(message);
  if (length == 1)
    return;
  p=message;
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if ((*p == '\'') || (*p == '"'))
    p++;
  q=message+length-1;
  while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
    q--;
  if (q > p)
    if ((*q == '\'') || (*q == '"'))
      q--;
  (void) CopyWizardMemory(message,p,(size_t) (q-p+1));
  message[q-p+1]='\0';
  for (p=message; *p != '\0'; p++)
    if (*p == '\n')
      *p=' ';
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S u b s t i t u t e S t r i n g                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SubstituteString() performs string substitution on a string, replacing the
%  string with the substituted version. Buffer must be allocated from the heap.
%  If the string is matched and status, WizardTrue is returned otherwise
%  WizardFalse.
%
%  The format of the SubstituteString method is:
%
%      WizardBooleanType SubstituteString(char **string,const char *search,
%        const char *replace)
%
%  A description of each parameter follows:
%
%    o string: the string to perform replacements on;  replaced with new
%      allocation if a replacement is made.
%
%    o search: search for this string.
%
%    o replace: replace any matches with this string.
%
*/
WizardExport WizardBooleanType SubstituteString(char **string,
  const char *search,const char *replace)
{
  WizardBooleanType
    status;

  register char
    *p;

  size_t
    extent,
    replace_extent,
    search_extent;

  ssize_t
    offset;

  status=WizardFalse;
  search_extent=0,
  replace_extent=0;
  for (p=strchr(*string,*search); p != (char *) NULL; p=strchr(p+1,*search))
  {
    if (search_extent == 0)
      search_extent=strlen(search);
    if (strncmp(p,search,search_extent) != 0)
      continue;
    /*
      We found a match.
    */
    status=WizardTrue;
    if (replace_extent == 0)
      replace_extent=strlen(replace);
    if (replace_extent > search_extent)
      {
        /*
          Make room for the replacement string.
        */
        offset=(ssize_t) (p-(*string));
        extent=strlen(*string)+replace_extent-search_extent+1;
        *string=(char *) ResizeQuantumMemory(*string,extent+MaxTextExtent,
          sizeof(*p));
        if (*string == (char *) NULL)
          ThrowFatalException(ResourceFatalError,
            "memory allocation failed `%s'");
        p=(*string)+offset;
      }
    /*
      Replace string.
    */
    if (search_extent != replace_extent)
      (void) CopyWizardMemory(p+replace_extent,p+search_extent,
        strlen(p+search_extent)+1);
    (void) CopyWizardMemory(p,replace,replace_extent);
    p+=replace_extent-1;
  }
  return(status);
}
