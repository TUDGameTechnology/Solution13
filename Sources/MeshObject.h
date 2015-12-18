#pragma once

#include <Kore/pch.h>


#include <Kore/Graphics/Graphics.h>

#include "ObjLoader.h"


namespace {
	class MeshObject {
	public:
		MeshObject(const char* meshFile, const char* textureFile, const Kore::VertexStructure& structure, float scale = 1.0f) {
			mesh = loadObj(meshFile);
			image = new Kore::Texture(textureFile, true);

			vertexBuffer = new Kore::VertexBuffer(mesh->numVertices, structure, 0);
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

			indexBuffer = new Kore::IndexBuffer(mesh->numFaces * 3);
			int* indices = indexBuffer->lock();
			for (int i = 0; i < mesh->numFaces * 3; i++) {
				indices[i] = mesh->indices[i];
			}
			indexBuffer->unlock();

			M = Kore::mat4::Identity();
		}

		void render(Kore::TextureUnit tex) {
			Kore::Graphics::setTexture(tex, image);
			Kore::Graphics::setVertexBuffer(*vertexBuffer);
			Kore::Graphics::setIndexBuffer(*indexBuffer);
			Kore::Graphics::drawIndexedVertices();
		}

		void setTexture(Kore::Texture* tex) {
			image = tex;
		}

		Kore::Texture* getTexture() {
			return image;
		}

		Kore::mat4 M;
	private:
		Kore::VertexBuffer* vertexBuffer;
		Kore::IndexBuffer* indexBuffer;
		Mesh* mesh;
		Kore::Texture* image;
	};

}