#pragma once

#include <Kore/Graphics4/Graphics.h>
#include "ObjLoader.h"

namespace {
	class MeshObject {
	public:
		MeshObject(const char* meshFile, const char* textureFile, const Kore::Graphics4::VertexStructure& structure, float scale = 1.0f) {
			mesh = loadObj(meshFile);
			image = new Kore::Graphics4::Texture(textureFile, true);
			
			vertexBuffer = new Kore::Graphics4::VertexBuffer(mesh->numVertices, structure, 0);
			float* vertices = vertexBuffer->lock();
			for (int i = 0; i < mesh->numVertices; ++i) {
				vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0] * scale;
				vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1] * scale;
				vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2] * scale;
				vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
				vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
				vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
				vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
				vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
			}
			vertexBuffer->unlock();
			
			indexBuffer = new Kore::Graphics4::IndexBuffer(mesh->numFaces * 3);
			int* indices = indexBuffer->lock();
			for (int i = 0; i < mesh->numFaces * 3; i++) {
				indices[i] = mesh->indices[i];
			}
			indexBuffer->unlock();
			
			M = Kore::mat4::Identity();
		}
		
		/** Mesh object from already loaded assets */
		MeshObject(Kore::Graphics4::VertexBuffer* inVertexBuffer, Kore::Graphics4::IndexBuffer* inIndexBuffer, Kore::Graphics4::Texture* inTexture)
		: vertexBuffer(inVertexBuffer), indexBuffer(inIndexBuffer), image(inTexture), M(Kore::mat4::Identity()) {
		}
		
		void render(Kore::Graphics4::TextureUnit tex) {
			Kore::Graphics4::setTexture(tex, image);
			Kore::Graphics4::setVertexBuffer(*vertexBuffer);
			Kore::Graphics4::setIndexBuffer(*indexBuffer);
			Kore::Graphics4::drawIndexedVertices();
		}
		
		void setTexture(Kore::Graphics4::Texture* tex) {
			image = tex;
		}
		
		Kore::Graphics4::Texture* getTexture() {
			return image;
		}
		
		Kore::Graphics4::VertexBuffer* getVertexBuffer()
		{
			return vertexBuffer;
		}
		
		Kore::Graphics4::IndexBuffer* getIndexBuffer()
		{
			return indexBuffer;
		}
		
		Kore::mat4 M;
		
	private:
		Kore::Graphics4::VertexBuffer* vertexBuffer;
		Kore::Graphics4::IndexBuffer* indexBuffer;
		Mesh* mesh;
		Kore::Graphics4::Texture* image;
	};
	
}
