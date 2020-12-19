# PlanetSphericalProjection
Research types of methods to project a texture/cubemap onto a sphere.

![Noise Cubemap](/media/Screenshot_376.png) ![Sphere](https://media.discordapp.net/attachments/563601152885653533/754965420699680799/unknown.png?width=936&height=479)
## Prelude
 The main purpose of this repo is to explore all possible ways to procedurally generate and project a cubemap onto a sphere. 
 I've chosen a cubemap as it has 6 sides, which is the much better than projecting a single image over a sphere.
 My intention is to have a cubemap for significant terrain features that are obvious when seen from space, extra noise will be applied onto the planet when it is viewed from a close distance. I also plan on using cubemaps for biome definitions.

## Issues

* Memory Usage

Generating textures and cubemaps for every planet is not ideal as typically a cubemap would 1024x512 or higher and having multiple of them (including generated 2D maps) could consume more than 500MB. I have several ideas on how to solve this problem and which the solutions will be added here.

## UE4 Plugin
I have included plugins for the methods I will be researching into. The numbers correspond to the projection method which is being applied.

## Method 1 - Spherical Texture - Mercator Projection
(example in the 2nd image on the heading) 
This is the method I am currently going with as UE4 stores cubemaps in this format. 

These following functions are available as material functions in UE4 and can be repliacted in the Voxel Graph.

### Projecting from
```cpp
// Take in a position, then normalize it
float3 n = normalize(position);
float u = ((atan2(n.x, -n.y)/PI)+1)/2;
float v = acos(n.z) / PI;

return float2(u,v);
```

### Projecting to:
```cpp
float2 Angles = (2*PI*(UV.x+0.5f), PI*UV.y);
float s= sin(Angles.y);
float Direction = float3(s*sin(Angles.x), cos(Angles.y), -s*cos(Angles.x));
return Direction.xzy;
```

## Resources to look at (to be extended)
https://pdfs.semanticscholar.org/0fcc/4445fd71b6e9e68ec7ae02a23b5720f4ded2.pdf

https://core.ac.uk/download/pdf/228552506.pdf

https://marlam.de/ecm/lambers12ecm.pdf

http://www.virtual-globe.info/documentation/projection.html

http://vterrain.org/Textures/spherical.html

http://paulbourke.net/miscellaneous/cubemaps/

http://alexcpeterson.com/2015/08/25/converting-a-cube-map-to-a-sphericalequirectangular-map/

Voxel Plugin Discord (discord.voxelplugin.com) - Great info is being shared around in this server.
