[ The files in this directory are to support the Wizard's Toolbox
  setup.exe-style installer package. The following are the instructions
  foro how to build a DLL-based distribution installer package using Visual C++
  7.0. ]

	   Steps for building VisualMagick Distribution Package

1) Install prerequisite software

  a) Download and install the Inno Setup Compiler from
      "http://www.jrsoftware.org/isinfo.php".

  b) Download and install ActiveState ActivePerl from
     "http://www.activestate.com/Products/ActivePerl/Download.html".


2) Open workspace windows\Wizard's Toolkit.sln by double-clicking from
   Windows Explorer or opening workpace via Visual C++ dialog.

   a) Build -> "Set Active Configuration" -> "All - Win32 Release" -> OK
   b) Build -> "Rebuild All"

3) Open windows\installer\Wizard's Toolkit.iss by double-clicking from
   Windows Explorer.

   a) File -> Compile
   b) Test install by clicking on green triangle

4) Distribution package is available as

     windows\bin\Window's Toolkit-1.0.2-dll.exe
