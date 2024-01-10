;  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
;  dedicated to making software imaging solutions freely available.
;
;  You may not use this file except in compliance with the License.  You may
;  obtain a copy of the License at
;
;    https://urban-warrior.org/WizardsToolkit/script/license.php
;
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.

; #pragma option -v+
; #pragma verboselevel 4
; #define DEBUG 1

#define  public WizardDynamicPackage 1
#include "scripts\body.isx"

#ifdef Debug
  #expr SaveToFile(AddBackslash(SourcePath) + "Wizard's Toolkit.isp")
#endif
