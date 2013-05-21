/*    dynamo:- Event driven molecular dynamics simulator 
 *    http://www.dynamomd.org
 *    Copyright (C) 2009  Marcus N Campbell Bannerman <m.bannerman@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 3 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <magnet/GL/shader/detail/shader.hpp>
#define STRINGIFY(A) #A

namespace magnet {
  namespace GL {
    namespace shader {
      /*! \brief A deffered rendering (G-Buffer) shader which
	billboards/raytraces cylinders.

	This shader provides an extremely fast method to render
	perfect spheres in OpenGL. This method appears to outperform
	even the most poorly tesselated spheres. Only the position
	of the sphere (the input type is GL_POINTS) is needed as
	input (The radius of the sphere is passed in through the
	iScale vertex attribute). A geometry shader then converts
	each POINT into two triangles as a square bilboard. When the
	billboard is rasterized into fragments, in the fragment
	shader, each fragment is used to ray trace a sphere within
	the billboard. Thus, we only draw the front face of the
	sphere, using the absolute minimum input data, only two
	triangles at the cost of a slightly expensive fragment
	shader and an additional (trivial) geometry shader stage.

	Anti-aliasing can be achieved by forcing the GL state to
	evaluate all samples of the fragments using the
	GL_ARB_sample_shading extension when available. Something
	like \code glEnable(SAMPLE_SHADING_ARB);
	glMinSampleShadingARB(1.0); \endcode will enable
	multisampling on the spheres when possible.

	A discussion of this technique is given in the excellent
	online GL book by Jason L. McKesson at \url
	http://www.arcsynthesis.org/gltut/index.html in the chapter
	on lies and IMPOSTORS.
      */
      class CylinderShader: public detail::Shader
      {
      public:
	CylinderShader()
	{ 
	  defines("unshaded") = "false";
	}

	virtual std::string initVertexShaderSource()
	{
	  return STRINGIFY(
uniform mat4 ViewMatrix;
uniform float global_scale;

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 4) in vec4 iOrientation;
layout (location = 5) in vec4 iScale;

out vec4 color;
out vec3 axis;
out float radius;
out float length;

vec3 qrot(vec4 q, vec3 v)
{ return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v); } 

void main()
{
  color = vColor;
  radius = iScale.x * global_scale * 0.5;
  length = iScale.y * global_scale * 0.5;
  //Generate an axis, which is rotated by the view matrix so that I
  //have it in screen space
  axis = (ViewMatrix * vec4(qrot(iOrientation, vec3(0,0,1)), 0.0)).xyz;
  gl_Position = ViewMatrix * vec4(vPosition.xyz, 1.0);
});
	}
	
	virtual std::string initGeometryShaderSource()
	{
	  return STRINGIFY(
uniform mat4 ProjectionMatrix;

layout(points) in;
)"\n#ifdef DRAWBILLBOARD\n"STRINGIFY(
layout(line_strip) out;
layout(max_vertices = 5) out;
)"\n#else\n"STRINGIFY(
layout(triangle_strip) out;
layout(max_vertices = 4) out;
)"\n#endif\n"STRINGIFY(

in vec4 color[];
in vec3 axis[];
in float radius[];
in float length[];

flat out vec4 vert_color;
flat out vec3 frag_axis;
flat out float frag_radius;
flat out float frag_length;
flat out vec3 sphere_center;
smooth out vec2 ordinate;

vec2 screen_perp;

//Function to emit a bilboard vertex with all the correct output given
//the displacement
void VertexEmit(in vec2 displacement)
{
  ordinate = displacement;
  
  vec4 proj_position = ProjectionMatrix 
    * (gl_in[0].gl_Position + vec4(length[0] * displacement.x * axis[0], 0.0)
       + vec4(radius[0] * displacement.y * screen_perp, 0.0, 0.0));
  gl_Position = proj_position;
  EmitVertex();
}

void main()
{
  //Standard data for each fragment
  vert_color = color[0];
  frag_radius = radius[0];
  frag_length = length[0];
  frag_axis = axis[0];
  sphere_center = gl_in[0].gl_Position.xyz;
  screen_perp = normalize(vec2(axis[0].y, -axis[0].x));
)"\n#ifdef DRAWBILLBOARD\n"STRINGIFY(
  VertexEmit(vec2(-1.0, -1.0));
  VertexEmit(vec2(-1.0, +1.0));
  VertexEmit(vec2(+1.0, +1.0));
  VertexEmit(vec2(+1.0, -1.0));
  VertexEmit(vec2(-1.0, -1.0));
)"\n#else\n"STRINGIFY(
  VertexEmit(vec2(-1.0, -1.0));
  VertexEmit(vec2(-1.0, +1.0));
  VertexEmit(vec2(+1.0, -1.0));
  VertexEmit(vec2(+1.0, +1.0));
)"\n#endif\n"STRINGIFY(
  EndPrimitive();
});
	}

	virtual std::string initFragmentShaderSource()
	{
	  return STRINGIFY(
uniform mat4 ProjectionMatrix;

flat in vec4 vert_color;
flat in vec3 frag_axis;
flat in float frag_radius;
flat in float frag_length;
flat in vec3 sphere_center;
smooth in vec2 ordinate;

layout (location = 0) out vec4 color_out;
layout (location = 1) out vec4 normal_out;
layout (location = 2) out vec4 position_out;

void main()
{
  vec3 billboard_frag_pos = sphere_center + vec3(ordinate, 0.0) * frag_radius;
  vec3 ray_direction = normalize(billboard_frag_pos);

  float TD = dot(ray_direction, -sphere_center);
  float c = dot(sphere_center, sphere_center) - frag_radius * frag_radius;
  float arg = TD * TD - c;
      
)"\n#ifndef DRAWBILLBOARD\n"STRINGIFY(
  if (arg < 0) discard;
)"\n#endif\n"STRINGIFY(
  
  float t = - c / (TD - sqrt(arg));

  vec3 frag_position_eye = ray_direction * t;
  
  //Calculate the fragments depth
  vec4 pos = ProjectionMatrix * vec4(frag_position_eye, 1.0);

)"\n#ifdef DRAWBILLBOARD\n"STRINGIFY(
  color_out = vert_color;
  normal_out = vec4(0.0);
  gl_FragDepth = 0; 
)"\n#else\n"STRINGIFY(
  gl_FragDepth = (pos.z / pos.w + 1.0) / 2.0; 
  //Write out the fragment's data
  position_out = vec4(frag_position_eye, 1.0);
  color_out = vert_color;
  if (unshaded)
    normal_out = vec4(0.0);
  else
    normal_out = vec4(normalize(frag_position_eye - sphere_center), 1.0);
)"\n#endif\n"STRINGIFY(
});
	}
      };

      /*! \brief A variant of the CylinderShader used for variance
          shadow mapping.
       */
      class CylinderVSMShader: public CylinderShader
      {
	virtual std::string initFragmentShaderSource()
	{
	  return STRINGIFY(
uniform mat4 ProjectionMatrix;

flat in float frag_radius;
flat in vec3 sphere_center;
smooth in vec2 ordinate;

layout (location = 0) out vec4 color_out;

void main()
{
  vec3 billboard_frag_pos = sphere_center + vec3(ordinate, 0.0) * frag_radius;
  vec3 ray_direction = normalize(billboard_frag_pos);

  float TD = dot(ray_direction, -sphere_center);
  float c = dot(sphere_center, sphere_center) - frag_radius * frag_radius;
  float arg = TD * TD - c;
      
  if (arg < 0) discard;
  
  float t = - c / (TD - sqrt(arg));

  vec3 frag_position_eye = ray_direction * t;

  //Calculate the fragments depth
  vec4 pos = ProjectionMatrix * vec4(frag_position_eye, 1.0);
  gl_FragDepth = (pos.z / pos.w + 1.0) / 2.0;

  float depth = -frag_position_eye.z;
  float A = ProjectionMatrix[2].z;
  float B = ProjectionMatrix[3].z;
  float moment1 = 0.5 * (-A * depth + B) / depth + 0.5;
  float moment2 = moment1 * moment1;

  // Adjusting moments (this is sort of bias per pixel) using derivative
  float dx = dFdx(moment1);
  float dy = dFdy(moment1);
  moment2 += 0.25 * (dx * dx + dy * dy);
	
  color_out = vec4(moment1, moment2, 0.0, 1.0);
});
	}

      };
    }
  }
}

#undef STRINGIFY