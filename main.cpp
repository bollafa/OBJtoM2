#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <set>

#include "m2.h"
#include "m2struct.h"

/*

TODO
-> Debug AnimationBlock : seems like offset are ok now, but.. Values are fucked up ? Why are there MD20 everywhere O'o, and Goblin..Male ?!!
-> Debug Calculation of Bones size. Seems like it's wrongly calculated, hoooow is it possible \o/.

*/

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

bool loadOBJ( const char * path, std::vector<Vec3D > & VertexMap, std::vector<Vec2D> & UVMap, std::vector<Vec3D> & NormalMap, std::vector<std::pair<uint32, std::array<int, 9> > > & Faces, uint32 & group_number, std::vector<std::string> & materials)
{
	std::ifstream objFile(path);
	
	if(!objFile.good())
		return false;

	std::string line;

	int VertexIndice = 0, UVIndice = 0, NormalIndice = 0, TriangleIndice = 0;
	group_number = 0;

	while(std::getline(objFile, line))
	{
		if(line.length() == 0)
			continue;

		std::vector<std::string> elem = split(line, ' ');
		
		if(elem[0] == "v")
		{
			if(elem.size() < 4)
			{
				std::cout << "Wrong format for v expected 4, got " << elem.size() << " for vertex " << VertexIndice << std::endl;
				return false;
			}

			Vec3D vertice;
			
			vertice.x = std::stof(elem[1]);
			vertice.y = std::stof(elem[2]);
			vertice.z = std::stof(elem[3]);

			VertexMap.push_back(vertice);
			VertexIndice++;
		}
		else if(elem[0] == "vt")
		{
			if(elem.size() < 3)
			{
				std::cout << "Wrong format for vt expected 3, got " << elem.size() << " for texture coord " << UVIndice << std::endl;
				return false;
			}

			Vec2D texture;

			texture.x = std::stof(elem[1]);
			texture.y = std::stof(elem[2]);

			UVMap.push_back(texture);
			UVIndice++;
		}
		else if(elem[0] == "vn")
		{
			if(elem.size() < 4)
			{
				std::cout << "Wrong format for vn expected 4, got " << elem.size() << " for normal " << NormalIndice << std::endl;
				return false;
			}

			Vec3D normal;
			
			normal.x = std::stof(elem[1]);
			normal.y = std::stof(elem[2]);
			normal.z = std::stof(elem[3]);

			NormalMap.push_back(normal);
			NormalIndice++;
		}
		else if(elem[0] == "f")
		{
			if(elem.size() < 4)
			{
				return false;
			}

			std::array<int, 9> face;

			for(int i = 0; i < 3; i++)
			{
				std::vector<std::string> vertexProperty = split(elem[i+1], '/');
				
				if(vertexProperty.size() < 3)
				{
					return false;
				}

				face[3*i] = std::stoi(vertexProperty[0]);
				face[3*i+1] = std::stoi(vertexProperty[1]);
				face[3*i+2] = std::stoi(vertexProperty[2]);
			}

			Faces.push_back(std::make_pair(group_number, face));
		}
		else if(elem[0] == "g")
		{
			group_number++; // New geoset !
			std::cout << "Geoset " << group_number << " parsing" << std::endl;
		}
		else if(elem[0] == "usemtl") // we may not need mtllib, we take material name as texture name
		{		
			materials.push_back(elem[1]);
			std::cout << "Geoset " << materials.size() << " will use material " << elem[1] << std::endl;
		}
		else
		{
			std::cout << "Unrecognized token during OBJ parsing : " << elem[0] << std::endl;
		}
	}

	std::cout << "Vertices " << VertexMap.size() << " Texture Coord " << UVMap.size() << " Normals " << NormalMap.size() << " Faces " << Faces.size() << std::endl;
	return true;
}

int main(int argc, char **argv)
{
	std::cout << "M2Lib | Converter v0.1\n" << "by Garthog\n" << "Thanks to relaxok, schlumpf, gamh, mjollna, zim for code / advices they gave me.\n" << "Thanks to pxr.dk & modcraft" << std::endl;

	std::vector<Vec3D> vertices;
	std::vector<Vec3D> normals;
	std::vector<Vec2D> UVs;
	std::vector<std::pair<uint32, std::array<int, 9> > > triangles;
	std::vector<std::string> materials;
	uint32 geoset_number = 0;

	m2 output;

	if(argc<3)
	{
		printf("Usage: OBJtoM2 <OBJFile> <ModelName> <TexturePath>\n");
		return 0;
	}

	/*
	output.loadFromFile(argv[1]);
	output.saveToFile(argv[2]);
	output.printSize();
	
	return 0;
	*/
	
	if(!loadOBJ(argv[1], vertices, UVs, normals, triangles, geoset_number, materials))
	{
		printf("Error loading OBJ file\n");
		return 1;
	}

	// Meta informations

	output.setName(argv[2]);
	output.setVersion(264);

	// Dummy things 

	output.AddDummyAnim();
	output.AddBone(0, 0, Vec3D());
	output.AddDummyTransparency();

	// M2 creation - Geometry
	
	output.AddSkin(20);

	/*
	std::cout << "Vertices" << std::endl;

	for(int i = 0; i < vertices.size(); i++)
	{
		std::cout << i << std::endl;
		Vec3D pos, normal;
		Vec2D uv;

		pos = vertices.at(i);
		normal = normals.at(i);
		uv = UVs.at(i);

		output.AddVertice(pos, normal, uv);
	}*/

	std::cout << "Adding triangles & vertice from faces" << std::endl;
	std::set<int> AddedTriangles;

	output.ReserveVertices(vertices.size());

	for(int i = 0; i < triangles.size(); i++)
	{
		std::array<int, 9> face = triangles.at(i).second;
		triangle tri;
		
		tri.indice1 = face[0] - 1;
		tri.indice2 = face[3] - 1;
		tri.indice3 = face[6] - 1;

		Vec3D pos, normal;
		Vec2D uv;

		for(int j = 0; j < 3; j++)
		{
			pos = vertices.at(face[j*3] - 1);
			uv = UVs.at(face[j*3+1] - 1);
			normal = normals.at(face[j*3+2] - 1);

			if(AddedTriangles.find(face[j*3]) == AddedTriangles.end())
			{
				AddedTriangles.insert(face[j*3]);

				// Flippping UV
				uv.y = 1 - uv.y;

				output.AddVertice(pos, normal, uv, face[j*3] - 1);
			}
		}
		
		output.AddTriangle(tri);
	}

	//output.FlipTex();

	// Submesh
	std::cout << "Generating Submeshes" << std::endl;

	uint16 startV = 0, startT = 0;
	std::array<int , 3> tri;

	if(geoset_number > 0)
	{
		for(int i = 1; i <= geoset_number; i++)
		{
			std::set<std::array<int, 3> > setTriangles;
			std::set<int> setVertices;

			// Comptage triangles / vertices
			for(int j = 0; j < triangles.size(); j++)
			{
				if(triangles.at(j).first == i)
				{		
					tri[0] = triangles.at(j).second[0] - 1;
					tri[1] = triangles.at(j).second[3] - 1;
					tri[2] = triangles.at(j).second[6] - 1;

					setVertices.insert(triangles.at(j).second[0] - 1);
					setVertices.insert(triangles.at(j).second[3] - 1);
					setVertices.insert(triangles.at(j).second[6] - 1);

					setTriangles.insert(tri);
				}
			}

			output.AddSubmesh(0, startV, setVertices.size(), startT, setTriangles.size() * 3);
			startV += setVertices.size();
			startT += setTriangles.size() * 3;
		}
	}
	else
	{
		output.AddSubmesh(0, 0, vertices.size(), 0, triangles.size() * 3);
	}

	// Materials unit
	std::cout << "Adding Materials" << std::endl;

	/*
	for(int i = 0; i < materials.size(); i++)
	{
		std::stringstream texname;
		texname << argv[3] << materials.at(i) << ".blp";

		output.AddTexture(texname.str(), 0);
		output.AddTextureUnit(i, i, i);
		output.AddRenderFlag(0, 0);
	}
	*/

	std::cout << "Command line utility\nPress \"i\" to get info about the model\nPress \"te\" to add a texture\nPress \"tu\" to add a textureUnit" 
		<< "\nPress \"r\" to add a renderflag\nPress \"q\" to exit and save file" << std::endl;
	std::string command;

	while(true)
	{
		std::cin >> command;

		if(command == "i")
		{
			std::cout << "There are " << output.getSkins()->Submeshes.size() << " submeshes" << std::endl;

			for(int i = 0; i < output.getSkins()->Submeshes.size(); i++)
			{
				std::cout << "Submesh " << i << std::endl;
				
				for(int j = 0; j < output.getSkins()->TextureUnits.size(); j++)
				{
					TextureUnit texu = output.getSkins()->TextureUnits.at(j);
					texture* text = output.getTextures();

					if(texu.submeshIndex == i || texu.submeshIndex2 == i)
					{
						std::cout << "-> Referenced in texUnit " << j << "( Texture " << texu.texture << " [" << text[texu.texture].Filename << "] | RenderFlag " << texu.renderFlag << " )\n" << std::endl;
					}
				}
			}
		}
		else if(command == "te")
		{
			std::string texName;

			std::cout << "Enter texture name : ";
			std::cin >> texName;

			output.AddTexture(texName.c_str(), 0);
		}
		else if(command == "tu")
		{
			uint16 submesh, texture, renderflag;
			std::string reflect;

			std::cout << "Which submesh would you like to choose ? ";
			std::cin >> submesh;

			std::cout << "What texture would you like to choose ? ";
			std::cin >> texture;

			std::cout << "What RenderFlag would you like to choose ? ";
			std::cin >> renderflag;

			std::cout << "Is it a reflect ( 'y' or 'n' ) ? ";
			std::cin >> reflect;

			if(reflect == "y")
			{
				output.AddTextureUnit(submesh, renderflag, texture, true);
			}
			else
			{
				output.AddTextureUnit(submesh, renderflag, texture);
			}
		}
		else if(command == "r")
		{
			uint16 blending, flag;

			std::cout << "Flag ";
			std::cin >> flag;

			std::cout << "Blending ";
			std::cin >> blending;

			output.AddRenderFlag(flag, blending);
		}

		else if(command == "q")
		{
			break;
		}
		else
		{
			std::cout << "Unknown command \"" << command << "\"" << std::endl;
		}
	}

	std::cout << "Saving file " << argv[2] << ".m2" << std::endl;

	std::stringstream filename;
	filename << argv[2] << ".m2";

	output.saveToFile(filename.str());

	return 0;
}
