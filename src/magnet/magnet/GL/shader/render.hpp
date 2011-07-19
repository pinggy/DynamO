/*    dynamo:- Event driven molecular dynamics simulator 
 *    http://www.marcusbannerman.co.uk/dynamo
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
#include <iostream>
#define STRINGIFY(A) #A

namespace magnet {
  namespace GL {
    namespace shader {
      /*! \brief An instancing shadow mapping/diffuse/ambient and specular shader
       * with per-pixel lighting for rendering scenes.
       */
      class RenderShader: public detail::Shader
      {
      public:
	virtual std::string initVertexShaderSource()
	{
	  return STRINGIFY(
////Quaternion mathmatics
//https://mollyrocket.com/forums/viewtopic.php?p=6154
uniform mat4 ShadowMatrix;
uniform vec3 lightPosition;

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vNormal;
attribute vec4 iOrigin;
attribute vec4 iOrientation;
attribute vec4 iScale;

varying vec4 ShadowCoord;
varying vec3 lightDir;
varying vec3 normal;
varying vec4 color;
varying vec3 eyeVector;


vec3 qrot(vec4 q, vec3 v) 
{ return v + 2.0 * cross(cross(v,q.xyz) + q.w * v, q.xyz); } 

void main()
{
  color = vColor;
  normal = normalize(gl_NormalMatrix * qrot(iOrientation, vNormal.xyz));
  
  vec4 vVertex = gl_ModelViewMatrix * vec4(qrot(iOrientation, vPosition.xyz * iScale.xyz) + iOrigin.xyz, 1.0);
  gl_Position = gl_ProjectionMatrix * vVertex;
  
  ShadowCoord = ShadowMatrix * vVertex;
  lightDir =  (gl_ModelViewMatrix * vec4(lightPosition,1)).xyz - vVertex.xyz;
  eyeVector = -vVertex.xyz;
});
	}
	
	virtual std::string initFragmentShaderSource()
	{
	  return STRINGIFY(
uniform sampler2DShadow ShadowMap; //The sampler for the shadow map
uniform int ShadowMapping; //If shadow mapping is enabled or not
uniform float ShadowIntensity; //How dark the shadow is
uniform float xPixelOffset;
uniform float yPixelOffset;

varying vec4 ShadowCoord; // Texture coordinate used for shadow lookup
varying vec3 lightDir; //Direction of the light
varying vec3 normal; //The surface normal
varying vec4 color;
varying vec3 eyeVector;


void main()
{
  ///Shadow map calculations and PCF.
  const int steps = 1;
  const float stepsf = float(steps);
  const float stepoffset = (stepsf - 1.0) * 0.5;
  
  //Flip the normal for 2 sided rendering and renormalize the normals again
  vec3 renormal = normalize(normal);  
  if (!gl_FrontFacing) renormal = -renormal;
  
  //Now get the diffuse term to smooth shadow acne on back faces
  vec3 renormLightDir = normalize(lightDir);
  float lightNormDot = dot(renormal, renormLightDir);

  //Shadow factor (0 = in shadow, 1 = unshadowed)
  //If shadow mapping is off, we want everything to be unshadowed
  float shadow = 1.0 - float(ShadowMapping);
  if (bool(ShadowMapping) //Perform shadow mapping if enabled
      && (ShadowCoord.w > 0.0) //Only check surfaces in front of the light
      )
    {
      vec4 ShadowCoordWdiv = ShadowCoord / ShadowCoord.w;
      for (int x = 0; x < steps; ++x)
	for (int y = 0; y < steps; ++y)
	  {
	    vec4 sampleCoords = ShadowCoordWdiv;
	    sampleCoords.x += (float(x) - stepoffset) * xPixelOffset;
	    sampleCoords.y += (float(y) - stepoffset) * yPixelOffset;

	    vec2 circle = (sampleCoords.xy) - vec2(0.5, 0.5);
	    
	    //Makes the light source a spotlight (and fixes a wierd
	    //line perpendicular to the light beam).
	    if (dot(circle, circle) < 0.25)
	      shadow += shadow2DProj(ShadowMap, sampleCoords).r;
	  }
      shadow /= stepsf * stepsf;
    }

  vec3 Eye = normalize(eyeVector);
  vec3 ReflectedRay = reflect(-renormLightDir, renormal);

  //Specular light
  float intensity = float(lightNormDot > 0.0) * shadow * pow(max(dot(ReflectedRay, Eye), 0.0), 25.0);

  //Ambient light
  intensity += 0.2;

  //Diffuse light
  float scaledShadow = (1.0 + ShadowIntensity * (shadow - 1.0));
  float rescaledDot = 0.5 * lightNormDot + 0.5;
  intensity += scaledShadow * rescaledDot * rescaledDot;

  //Light attenuation
  float lightDist = length(lightDir);
  float attenuation = 1.0 / (1.0 + lightDist * (0.0 + 0.2 * lightDist));
  intensity *= attenuation;

  gl_FragColor = vec4(intensity * color.rgb, color.a);
});
	}
      };
    }
  }
}

#undef STRINGIFY
