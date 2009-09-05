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

  Wizard's Toolkit utility methods.
*/
#ifndef _WIZARDSTOOLKIT_UTILITY_H
#define _WIZARDSTOOLKIT_UTILITY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedPath,
  FilesystemPath,
  RootPath,
  HeadPath,
  TailPath,
  BasePath,
  ExtensionPath,
  SubnodePath,
  CanonicalPath
} PathType;

extern WizardExport char
  *Base64Encode(const unsigned char *,const size_t,size_t *),
  **GetPathComponents(const char *,unsigned long *);

extern WizardExport const char
  *ParseWizardTime(const char *,time_t *);

extern WizardExport FILE
  *WizardOpenStream(const char *,const char *);

extern WizardExport unsigned char
  *Base64Decode(const char *, size_t *);

extern WizardExport void
  AppendFileExtension(const char *,char *),
  ChopPathComponents(char *,const unsigned long),
  GetPathComponent(const char *,PathType,char *);

extern WizardExport WizardBooleanType
  GetExecutionPath(char *,const size_t),
  IsAccessible(const char *),
  IsWizardTrue(const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
