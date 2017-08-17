#pragma once
#include "stdafx.h"

#include <array>
#include <map>
extern void error_callback(int error, const char* description);

using std::vector;
using std::array;
using std::string;
using std::map;
struct objObject
{


	//File Data
	string name;
	vector<array<float,3>> vertexArray;
	vector<array<float, 2>> textureArray;
	vector<array<float, 3>> normalArray;
	vector<array<array<float, 3>, 3>> indexArray;

	// Ordered data
	map<array<float,3>, int> mapp;

	vector<array<float,8>> compiledVertex;
	vector<GLuint> indexList;

	void consolidate()
	{

		array<float, 3> *v3Ref_Vert {nullptr}, *v3Ref_Norm {nullptr};
		array<float, 2> *v2Ref {nullptr};

		for (array<array<float, 3>, 3> &triangleIndex : indexArray)
		{
			for (array<float, 3> &triIndexVert : triangleIndex)
			{
				if (mapp.find(triIndexVert) == mapp.end())
				{
					v3Ref_Vert = &vertexArray.at(triIndexVert[0]-1);
					v2Ref = &textureArray.at(triIndexVert[1]-1 );
					v3Ref_Norm = &normalArray.at(triIndexVert[2]-1 );

					compiledVertex.push_back({v3Ref_Vert[0][0], v3Ref_Vert[0][1], v3Ref_Vert[0][2], v2Ref[0][0], v2Ref[0][1], v3Ref_Norm[0][0], v3Ref_Norm[0][1], v3Ref_Norm[0][2]});

					mapp[triIndexVert] = compiledVertex.size()-1;
					indexList.push_back(compiledVertex.size()-1);
				}
				else
				{
					indexList.push_back(mapp[triIndexVert]);
				}
			}


		}
	}

};

struct objFile
{



};



char getMode(std::string &line, int &length)
{

	if (length < 2)
	{
		return '\0';
	}
	char secondChar {line[1]};

	switch (line[0])
	{
		case 'v':
			if(secondChar == 't')
				return 't';
			if (secondChar == 'n')
				return 'n';
			return 'v';
			break;

		case 'f':
			if (secondChar == ' ')
				return 'f';
			break;

		case 'o':
			if (secondChar == ' ')
				return 'o';
			break;
		default:
			return '\0';
			break;
	}
}


bool loadObject(std::string objFileName, std::vector<objObject> &objectVector)
{
	std::ifstream objFile(objFileName.c_str(), std::ios::in);

	if (!objFile.good())
	{
		error_callback(0, "Failed to open obj model file!\n");
		return false;
	}


	std::string line, numberBuffer;

	char firstChar{'\0'}, mode{'\0'};
	int lineLen{0}, vertIndex{0}, polyIndex{0}, charIndex{0};


	array<array<float, 3>, 3> VertexData;
	objObject curObj;

	while (getline(objFile, line))
	{

		vertIndex = 0;
		polyIndex = 0;
		lineLen = line.length();

		// Check to see if the file line is a vertex, texture vertex, normal, or object name.
		mode = getMode(line, lineLen);
		if (mode == '\0')
		{
			continue;
		}
		
		// We found the start of an object defined by its name.
		// Add the name to the class, and continue onward.
		if (mode == 'o')
		{
			if (!curObj.name.empty())
			{
				objectVector.push_back(std::move(curObj));
			}
			curObj = objObject();
			curObj.name = line.substr(2,line.find(':')-2);
			
			continue;
		}

		// Parse the line character by character for vertex/texture coord/index/normal data.
		for (charIndex = 0; charIndex <= lineLen; charIndex++)
		{
			firstChar = line[charIndex];

			// The number buffer is considered complete when these conditions are met.
			if ( (firstChar == ' ' || firstChar == '/' || charIndex == lineLen) && !numberBuffer.empty())
			{



				switch (mode)
				{
					case 'v':
					case 't':
					case 'n':
						VertexData[0][vertIndex++] = ::atof(numberBuffer.c_str());
						break;

					case 'f':
						VertexData[polyIndex][vertIndex++] = ::atoi(numberBuffer.c_str());
						if (vertIndex == 3)
						{
							vertIndex = 0;
							polyIndex++;
						}
						if (polyIndex == 3)
						{
							polyIndex = 0;
						}
						break;
					default:
						error_callback(0, "Unknown object vertex/texture/normal/index data!!!\n");
					break;
				}
					
				numberBuffer.clear();
			}
			// The number buffer is still incomplete, so let's see if the current character is valid to add.
			else if (isdigit(firstChar) || firstChar == '.' || firstChar == '-')
			{
				numberBuffer.push_back(firstChar);
			}
		}
		// The number buffer was completed and the data used.
		// Ready the numberBuffer for the next number.
		numberBuffer.clear();

		switch (mode)
		{
			case 'v':
				curObj.vertexArray.push_back(VertexData[0]);
				break;
			case 't':
				curObj.textureArray.push_back(std::array<float, 2>{VertexData[0][0], VertexData[0][1]});
				break;
			case 'n':
				curObj.normalArray.push_back(VertexData[0]);
				break;
			case 'f':
				curObj.indexArray.push_back(VertexData);
				break;
			default:
				error_callback(0,"Unknown pushback vertex/texture/normal/index data!!!\n");
		}
	}
	objectVector.push_back(std::move(curObj));
	return true;
}

class objLoaderTest: public ::testing::Test
{

	virtual void SetUp()
	{
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown()
	{
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	// Objects declared here can be used by all tests in the test case for Foo.
};

TEST_F(objLoaderTest, CanLoadBread)
{
	vector<objObject> obj;
	EXPECT_TRUE(loadObject(std::string(DIR_ROOT)+"data\\mesh\\rawBread.obj", obj) );
	EXPECT_TRUE(obj.size() == 2 );
	obj[0].consolidate();
	return;
}
