/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit version and copyright.
*/
#ifndef _WIZARDSTOOLKIT_VERSION_H
#define _WIZARDSTOOLKIT_VERSION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Define declarations.
*/
#define WizardPackageName "WizardsToolkit"
#define WizardCopyright  "(C) 1999 ImageMagick Studio LLC"
#define WizardLibVersion  0x109
#define WizardLibVersionText  "1.0.9"
#define WizardLibVersionNumber  1,0,1
#define WizardLibAddendum  "-5"
#define WizardLibInterface  1
#define WizardLibMinInterface  1
#define WizardReleaseDate  "2022-02-26"
#define WizardChangeDate   "@PACKAGE_CHANGE_DATE@"
#define WizardAuthoritativeURL  "http://www.wizards-toolkit.org"
#define WizardVersion WizardPackageName " " WizardLibVersionText \
  WizardLibAddendum " " WizardReleaseDate " " WizardAuthoritativeURL

/*
  Method declarations.
*/
extern WizardExport const char
  *GetWizardCopyright(void),
  *GetWizardVersion(size_t *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
