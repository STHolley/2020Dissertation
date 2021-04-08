#version 450 core

uniform mat4 modelMatrix = mat4(1.0f);
uniform mat4 viewMatrix = mat4(1.0f);
uniform mat4 projMatrix = mat4(1.0f);

uniform int chunkSize = 16;
uniform int maxChunkSize = 8;

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

layout(std430, binding = 0) buffer triTable
{
    int data_SSBO[256][16];
};

in Vertex {
	vec4 colour;
	vec3 worldPos;
	vec2 texCoord;
	vec3 normal;
} IN[];

out Vertex
{
	vec4 colour;
	vec3 worldPos;
	vec2 texCoord;
	vec3 normal;
} outValue;

int[16] initTable(int index){
	return data_SSBO[index];
}

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
	
	const vec2  C = vec2(1.0 / 6.0, 1.0 / 3.0);
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i = floor(v + dot(v, C.yyy));
	vec3 x0 = v - i + dot(i, C.xxx);

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min(g.xyz, l.zxy);
	vec3 i2 = max(g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

  // Permutations
	i = mod289(i);
	vec4 p = permute(permute(permute(
		i.z + vec4(0.0, i1.z, i2.z, 1.0))
		+ i.y + vec4(0.0, i1.y, i2.y, 1.0))
		+ i.x + vec4(0.0, i1.x, i2.x, 1.0));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)

	vec4 x = x_ * ns.x + ns.yyyy;
	vec4 y = y_ * ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4(x.xy, y.xy);
	vec4 b1 = vec4(x.zw, y.zw);

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0) * 2.0 + 1.0;
	vec4 s1 = floor(b1) * 2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
	vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

	vec3 p0 = vec3(a0.xy, h.x);
	vec3 p1 = vec3(a0.zw, h.y);
	vec3 p2 = vec3(a1.xy, h.z);
	vec3 p3 = vec3(a1.zw, h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
	m = m * m;
	return (0.0 + (42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1),
		dot(p2, x2), dot(p3, x3)))));
}

float fbm(vec3 p, float lac)
{
	p *= lac;
	float f = snoise(p);
	return f*0.5+0.5;
}

void main(void)
{
	mat4 mvp = (projMatrix * viewMatrix * modelMatrix);

	for(int i = 0; i < gl_in.length(); i++){ //Each point entered in

		vec3 start = IN[i].worldPos;

		float blockHeight[4] = float[4](fbm(vec3(start.xz,1), 0.01), fbm(vec3(start.x + 1, start.z, 1), 0.01), fbm(vec3(start.x + 1, start.z + 1, 1), 0.01), fbm(vec3(start.x, start.z + 1, 1), 0.01));
		float baseHeight[4] = float[4](fbm(vec3(start.xz, 0), 0.01), fbm(vec3(start.x + 1, start.z, 0), 0.01), fbm(vec3(start.x + 1, start.z + 1, 0), 0.01), fbm(vec3(start.x, start.z + 1, 0), 0.01));

		float cube[8];

		vec3 block[8] = vec3[8](start, vec3(start.x + 1, start.y, start.z), vec3(start.x + 1, start.y, start.z + 1), vec3(start.x, start.y, start.z + 1),
			vec3(start.x, start.y + 1, start.z), vec3(start.x + 1, start.y + 1, start.z), vec3(start.x + 1, start.y + 1, start.z + 1), vec3(start.x, start.y + 1, start.z + 1));

		for (int j = 0; j < 4; j++) {
			blockHeight[j] = (chunkSize * maxChunkSize * 0.5) + (blockHeight[j] * (chunkSize * maxChunkSize * 0.5));
			baseHeight[j] = 1 + ((0.5 * maxChunkSize) * baseHeight[j]);
		}

		for(int j = 0; j < 8; j++){
			if(block[j].y > blockHeight[int(mod(j,4))]){
				cube[j] = 0;
			}
			if (block[j].y <= blockHeight[int(mod(j,4))]) {
				cube[j] = fbm(block[j], 0.01);
			}
			if(block[j].y <= baseHeight[int(mod(j,4))]){
				cube[j] = 1;
			}
		}

		float floorVal = 0.4;
		int shifter = 0;
		int cubeIndex = 0;
		for (int i = 0; i < 8; i++) {
			if (cube[i] >= floorVal) {
				cubeIndex |= (1 << shifter);
			}
			shifter++;
		}
		int pointCount = 0;
		vec3 tri[3] = vec3[3](vec3(0,0,0), vec3(0,0,0), vec3(0,0,0));
		int triList[16] = initTable(cubeIndex);// initList(cubeIndex);

		for (int j = 0; j < 16; j++) { //For all results in the tri table
			int val = triList[j];
			if(val != -1){
				vec3 pos = IN[i].worldPos;
				vec3 change = vec3(0, 0, 0);
				switch (val) { //Get the "Position" and turn it into a real 3D coord
				case(0):
					change = vec3(0.5, 0, 0);
					break;
				case(1):
					change = vec3(1, 0, 0.5);
					break;
				case(2):
					change = vec3(0.5, 0, 1);
					break;
				case(3):
					change = vec3(0, 0, 0.5);
					break;
				case(4):
					change = vec3(0.5, 1, 0);
					break;
				case(5):
					change = vec3(1, 1, 0.5);
					break;
				case(6):
					change = vec3(0.5, 1, 1);
					break;
				case(7):
					change = vec3(0, 1, 0.5);
					break;
				case(8):
					change = vec3(0, 0.5, 0);
					break;
				case(9):
					change = vec3(1, 0.5, 0);
					break;
				case(10):
					change = vec3(1, 0.5, 1);
					break;
				case(11):
					change = vec3(0, 0.5, 1);
					break;
				}
				tri[pointCount] = (pos + change);
				pointCount++;
				if(pointCount == 3){
					
					vec3 a = tri[0];
					vec3 b = tri[1];
					vec3 c = tri[2];
					a.y = -a.y;
					b.y = -b.y;
					c.y = -c.y;
					vec3 norm = cross(b - a, c - a);
					norm = normalize(norm);
					a.y = -a.y;
					b.y = -b.y;
					c.y = -c.y;
					gl_Position = mvp * vec4(a, 1.0);
					outValue.worldPos = gl_Position.xyz;
					outValue.normal = norm;
					outValue.texCoord = vec2(0, 0);
					outValue.colour = IN[i].colour;
					EmitVertex();

					gl_Position = mvp * vec4(b, 1.0);
					outValue.worldPos = gl_Position.xyz;
					outValue.normal = norm;
					outValue.texCoord = vec2(1, 0);
					outValue.colour = IN[i].colour;
					EmitVertex();

					gl_Position = mvp * vec4(c, 1.0);
					outValue.worldPos = gl_Position.xyz;
					outValue.normal = norm;
					outValue.texCoord = vec2(0.5, 1);
					outValue.colour = IN[i].colour;
					EmitVertex();

					EndPrimitive();
					pointCount = 0;
				}
			}
		}	
	}
	EndPrimitive();
}

