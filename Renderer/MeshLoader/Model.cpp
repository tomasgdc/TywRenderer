/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "Model_local.h"
#include "Model_obj.h"
#include "ImageManager.h"
#include "Material.h"
#include "VKRenderer.h"

//Geometry data
#include "Geometry\VertData.h"

//Vulkan Includes
#include "Vulkan\VulkanTextureLoader.h"



/*
=========================
RenderModelStatic
=========================
*/
RenderModelStatic::RenderModelStatic() {

}

/*
=========================
InitFromFile
=========================
*/
void RenderModelStatic::InitFromFile(std::string fileName, std::string filePath) {
	bool loaded = false;

	loaded = OBJLoad(fileName, filePath);
	if (!loaded) 
	{
		printf("Could not load model: %s \n", fileName.c_str());
		assert(loaded && "Could not load model");
	}
}

/*
=========================
OBJLoad
=========================
*/
bool RenderModelStatic::OBJLoad(std::string& fileName, std::string& filePath) {
	//DEBUG
	//engine->Sys_Printf("Loading OBJ model: %s \r\n", fileName.c_str());
	printf("Loading OBJ model %s \n", fileName.c_str());

	objModel_t* obj;
	obj = OBJ_Load(fileName, filePath);
	if (obj == nullptr) {
		return false;
	}
	ConvertOBJToModelSurfaces(obj);
	OBJ_Free(obj);
	return true;
}




void CalculateNormals(objMesh_t& mesh, srfTriangles_t & tri)
{
	glm::vec3 sumNormals;
	for (uint32_t i = 0; i < mesh.numFaces * 3; i++)
	{
		glm::vec3& normal = tri.verts[i].normal;
		sumNormals = normal;
		for (uint32_t j = i+1; j < mesh.numFaces*3; j++)
		{
			if (tri.verts[i].vertex == tri.verts[j].vertex)
			{
				sumNormals += tri.verts[j].normal;
			}
		}
		normal = glm::normalize(sumNormals);
	}
}

/*
=========================
ConvertOBJToModelSuraces
=========================
*/
bool RenderModelStatic::ConvertOBJToModelSurfaces(const struct objModel_a* obj) {
	int					objectNum;
	int					materialNum;
	srfTriangles_t *	tri;
	objObject_t*		object;
	objMaterial_t*		material;
	objMesh_t*			mesh;
	modelSurface_t		surf, *modelSurf;

	//Stores number of materials loaded. Hack around till will figure out how to go around this problem.
	std::unordered_map<std::string, uint32_t> materialSizes;

	//Loads image if it was not stored in globalImage
	for (materialNum = 0; materialNum < obj->materials.size(); materialNum++) 
	{
		material = obj->materials[materialNum];

		//Create new material
		Material* mat = TYW_NEW Material[material->count];
		uint32_t matNumber = 0;

		if (strlen(material->map_bump) != 0) {
			//globalImage->GetImage(material->map_bump, VkFormat::VK_FORMAT_UNDEFINED);
			
			mat[matNumber].setTexture(globalImage->GetImage(material->map_bump, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		if (strlen(material->map_d) != 0) {
			//globalImage->GetImage(material->map_d, VkFormat::VK_FORMAT_UNDEFINED);

			mat[matNumber].setTexture(globalImage->GetImage(material->map_d, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		if (strlen(material->map_Ka) != 0) {
			//globalImage->GetImage(material->map_Ka, VkFormat::VK_FORMAT_UNDEFINED);

			mat[matNumber].setTexture(globalImage->GetImage(material->map_Ka, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		if (strlen(material->map_Kd) != 0) {
			//globalImage->GetImage(material->map_Kd, VkFormat::VK_FORMAT_UNDEFINED);

			mat[matNumber].setTexture(globalImage->GetImage(material->map_Kd, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		if (strlen(material->map_Ks) != 0) {
			//globalImage->GetImage(material->map_Ks, VkFormat::VK_FORMAT_UNDEFINED);

			mat[matNumber].setTexture(globalImage->GetImage(material->map_Ks, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		if (strlen(material->map_Ns) != 0) {
			//globalImage->GetImage(material->map_Ns, VkFormat::VK_FORMAT_UNDEFINED);

			mat[matNumber].setTexture(globalImage->GetImage(material->map_Ns, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			matNumber++;
		}
		m_material.insert(std::pair<std::string, Material*>(material->name, mat));
		materialSizes.insert(std::pair<std::string, uint32_t>(material->name, matNumber));
	}

	//Reserve needed size for surfaces
	surfaces.reserve(obj->objects.size());

	//Converts OBJ mesh data to modelSurface_t data
	for (objectNum = 0; objectNum < obj->objects.size(); objectNum++) 
	{
		surfaces.push_back(surf);
		object = obj->objects[objectNum];
		mesh = &object->mesh;


		tri = nullptr;
		tri = R_AllocStaticTriSurf();
		tri->numVerts = 0;
		tri->numIndexes = 0;
		tri->indexes = nullptr;

		R_AllocStaticTriSurfVerts(tri, mesh->numFaces * 3);
		for (int i = 0; i < mesh->numFaces * 3; i+=3) 
		{
			tri->verts[i].Clear();
			tri->verts[i + 1].Clear();
			tri->verts[i + 2].Clear();

			//Firts vertex
			tri->verts[i].vertex = mesh->vertices[i];
			tri->verts[i].normal = mesh->normals[i];
			tri->verts[i].tangent = mesh->tangent[i];
			tri->verts[i].bitangent = mesh->binormal[i];
			tri->verts[i].tex = mesh->uvs[i];

			
			//Second vertex
			tri->verts[i + 1].vertex = mesh->vertices[i + 1];
			tri->verts[i + 1].normal = mesh->normals[i+1];
			tri->verts[i + 1].tangent = mesh->tangent[i+1];
			tri->verts[i + 1].bitangent = mesh->binormal[i+1];
			tri->verts[i + 1].tex = mesh->uvs[i+1];

			//Third vector
			tri->verts[i + 2].vertex = mesh->vertices[i + 2];
			tri->verts[i + 2].normal = mesh->normals[i+2];
			tri->verts[i + 2].tangent = mesh->tangent[i+2];
			tri->verts[i + 2].bitangent = mesh->binormal[i+2];
			tri->verts[i + 2].tex = mesh->uvs[i + 2];
		}

		modelSurf = &this->surfaces[objectNum];
		modelSurf->geometry = tri;
		modelSurf->material = nullptr;
		it = m_material.find(object->mat_name);
		if (it != m_material.end()) 
		{
			Material * pMaterial = it->second;
			modelSurf->material = pMaterial;
			modelSurf->numMaterials = materialSizes[object->mat_name]; //Will give number of materials that were loaded
			printf("Found %s \n", object->mat_name);
		}
	}
	return true;
}

/*
=========================
addSurface
=========================
*/
void RenderModelStatic::addSurface(modelSurface_t& surface) {
	surfaces.push_back(surface);
}

/*
=========================
LoadModel
=========================
*/
void RenderModelStatic::LoadModel() {

}

/*
=========================
AllocSurfaceTriangles
=========================
*/
srfTriangles_t * RenderModelStatic::AllocSurfaceTriangles(int numVerts, int numIndexes) const {
	srfTriangles_t* tri = R_AllocStaticTriSurf();
	R_AllocStaticTriSurfVerts(tri, 6);
	return tri;
}

/*
=========================
InstantiateDynamicModel
=========================
*/
RenderModel *	RenderModelStatic::InstantiateDynamicModel() {
	//engine->Sys_Error("ERROR: InstantiateDynamicModel -> called on static model %s\r\n", name.c_str());
	return nullptr;
}

/*
=========================
setName
=========================
*/
void RenderModelStatic::setName(std::string name) {
	this->name = name;
}


/*
=========================
Memcpy
=========================
*/
void RenderModelStatic::Memcpy(srfTriangles_t * tri, size_t size) {
	//memcpy(m_geometry, tri, size);
}

/*
=========================
getName
=========================
*/
const char	* RenderModelStatic::getName() const {
	return name.c_str();
}

/*
=========================
getSize
=========================
*/
int RenderModelStatic::getSize() const {
	return 0;
}

/*
=========================
FreeSurfaceTriangles
=========================
*/
void RenderModelStatic::FreeSurfaceTriangles(srfTriangles_t *tris) {
	R_FreeStaticTriSurf(tris);
}


/*
=========================
Clear
=========================
*/
void RenderModelStatic::Clear() {
	std::vector<modelSurface_t>::iterator			m_it;
	for (m_it = surfaces.begin(); m_it != surfaces.end(); ++m_it) {
		FreeSurfaceTriangles(m_it->geometry);
	}
	surfaces.clear();

	for (it = m_material.begin(); it != m_material.end(); ++it) {
		it->second->Clear();
		SAFE_DELETE(it->second);
	}
	m_material.clear();
}


/*
=========================
setMaterial
=========================
*/
void RenderModelStatic::setMaterial(std::string name, Material* mat) {
	m_material.insert(std::map<std::string, Material*>::value_type(name, mat));
}