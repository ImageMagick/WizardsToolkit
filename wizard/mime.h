/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The Wizard's Toolkit mime methods.
*/
#ifndef _WIZARDSTOOLKIT_MIME_H
#define _WIZARDSTOOLKIT_MIME_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MimeInfo
  MimeInfo;

extern WizardExport char
  **GetMimeList(const char *,unsigned long *,ExceptionInfo *),
  *WizardToMime(const char *);

extern WizardExport const char
  *GetMimeDescription(const MimeInfo *),
  *GetMimeType(const MimeInfo *);

extern WizardExport WizardBooleanType
  InstantiateMimeComponent(void),
  ListMimeInfo(FILE *,ExceptionInfo *),
  LoadMimeLists(const char *,ExceptionInfo *);

extern WizardExport const MimeInfo
  *GetMimeInfo(const char *,const unsigned char *,const size_t,ExceptionInfo *),
  **GetMimeInfoList(const char *,unsigned long *,ExceptionInfo *);

extern WizardExport void
  DestroyMimeComponent(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
