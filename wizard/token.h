/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit token methods.
*/
#ifndef _WIZARDSTOOLKIT_TOKEN_H
#define _WIZARDSTOOLKIT_TOKEN_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _TokenInfo
  TokenInfo;

extern WizardExport int
  Tokenizer(TokenInfo *,unsigned,char *,size_t,char *,char *,char *,char *,
    char,char *,int *,char *);

extern WizardExport WizardBooleanType
  GlobExpression(const char *,const char *,const WizardBooleanType),
  IsGlob(const char *);

extern WizardExport void
  GetNextToken(const char *,const char **,const size_t extent,char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
