/*
	Stanford Bunny (culled)
	Source: Stanford University Computer Graphics Laboratory
	
	You are welcome to use the data and models for research purposes. 
	You are also welcome to mirror or redistribute them for free, with 
	appropriate credit given. However, they are not to be used for 
	commercial purposes, nor should they appear in a product for sale, 
	without Stanford University Computer Graphics Laboratory permission.
	
	Scanner: Cyberware
	Number of scans: 10
	Total size of scans: 362,272 points (about 725,000 triangles) 
	Reconstruction: zipper 
	Size of culled reconstruction: 8146 vertices, 16301 triangles
	Comments: contains 5 holes in the bottom	

	See <http://graphics.stanford.edu/data/3Dscanrep/> for original data
*/

#include <OpenGL/gl.h>


GLuint GenStanfordBunnyWireList();
GLuint GenStanfordBunnySolidList();