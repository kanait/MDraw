# MDraw

MDraw is a research-purpose software for drawing lines on a mesh, which includes an implementation of the following papers:

Takashi Kanai, Hiromasa Suzuki:
"Approximate Shortest Path on a Polyhedral Surface and Its Applications",
Computer Aided Design, Volume 33, Number 11, pp.801-811, September 2001.

Takashi Kanai, Hiromasa Suzuki:
"Approximate Shortest Path on a Polyhedral Surface Based on Selective Refinement of the Discrete Graph and Its Applications",
Proc. Geometric Modeling and Processing 2000, pp.241-250, IEEE CS Press, Los Alamitos, CA, April 2000.

This software was originally developed in 1997-2000 and was renovated in 2016 so as to build successfully by Visual Studio 2015 (VS2015).

## Getting Started

### Windows

Double-click src\OpenGL.sln (A solution file for VS2015) and then "build the solution", and if successfully finished, you can find an executable in src\Release\OpenGL.exe .

### Unix (command-line version)

Makefile is included in src directory. Then, enter the src directory and type "make linux" or "make cygin".

## Prerequisites

This software uses MFC (Microsoft Foundation Class). Then, to build this software, VS2015 Professional or upper versions are required for Windows application.

For command-line version, there is no prerequities to build the binary.

## Authors

* **[Takashi Kanai](https://graphics.c.u-tokyo.ac.jp/hp/en/)** - The University of Tokyo

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

