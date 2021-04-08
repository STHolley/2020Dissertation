/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <string>
using std::string;
namespace NCL {
	namespace Rendering {
		enum ShaderStages {
			SHADER_VERTEX,
			SHADER_FRAGMENT,
			SHADER_GEOMETRY,
			SHADER_DOMAIN,
			SHADER_HULL,
			SHADER_MAX
		};

		class ShaderBase	{
		public:
			ShaderBase() {
			}
			ShaderBase(const string& vertex, const string& fragment, const string& geometry = "", const string& domain = "", const string& hull = "");
			virtual ~ShaderBase();

			virtual void ReloadShader() = 0;
		protected:

			string shaderFiles[ShaderStages::SHADER_MAX];
		};
	}
}

