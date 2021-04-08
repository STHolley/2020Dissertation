/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector4.h"
#include "Vector3.h"
using namespace NCL;
using namespace Maths;

Vector3 Vector4::ToVector3() const {
	return Vector3(x, y, z);
}