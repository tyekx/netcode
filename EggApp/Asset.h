#pragma once

#include <Egg/Modules.h>
#include <Egg/Asset/Model.h>
#include "Model.h"
#include <map>
#include <string>

std::map<std::string, std::string> GetPreprocDefs(Egg::Asset::Mesh * mesh, Egg::Asset::Material * material, Egg::Asset::Model * owner) {

	std::map<std::string, std::string> s;

	s["SHADER_CB_USE_PERMESH"];
	s["SHADER_CB_USE_PERFRAME"];
	s["SHADER_CB_USE_PEROBJECT"];
	s["SHADER_NUM_LIGHTS"] = std::to_string(1);

	if(mesh->vertexType == Egg::PNT_Vertex::type ||
	   mesh->vertexType == Egg::PNTWB_Vertex::type ||
	   mesh->vertexType == Egg::PNTTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
		s["IAO_HAS_NORMAL"];
		s["IAO_HAS_TEXCOORD"];
	}

	if(mesh->vertexType == Egg::PNTWBTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWB_Vertex::type) {
		s["IAO_HAS_SKELETON"];
		s["SHADER_CB_USE_BONEDATA"];
	}

	if(mesh->vertexType == Egg::PNTTB_Vertex::type ||
	   mesh->vertexType == Egg::PNTWBTB_Vertex::type) {
		s["IAO_HAS_TANGENT_SPACE"];
	}

	if(material->HasDiffuseTexture()) {
		s["SHADER_TEX_DIFFUSE"];
	}

	if(material->HasNormalTexture()) {
		s["SHADER_TEX_NORMAL"];
	}

	return s;
}

void LoadItem(Egg::Module::IGraphicsModule * g, Egg::Asset::Model * model, Model* modelComponent) {
	for(unsigned int meshIdx = 0; meshIdx < model->meshesLength; ++meshIdx) {
		Egg::Asset::Material * mat = model->materials + model->meshes[meshIdx].materialId;
		Egg::Asset::Mesh * mesh = model->meshes + meshIdx;

		Egg::HITEM item = g->CreateItem();
		Egg::HGEOMETRY geometry = g->CreateGeometry(Egg::EGeometryType::INDEXED);

		switch(mesh->vertexType) {
			case Egg::PNT_Vertex::type:
			{
				g->AddInputElement(geometry, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);
				g->AddInputElement(geometry, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12);
				g->AddInputElement(geometry, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24);
			}
			break;
			case Egg::PNTWB_Vertex::type:
			{
				g->AddInputElement(geometry, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);
				g->AddInputElement(geometry, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12);
				g->AddInputElement(geometry, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24);
				g->AddInputElement(geometry, "WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				g->AddInputElement(geometry, "BONEIDS", DXGI_FORMAT_R32G32B32A32_SINT, 44);
			}
			break;
			case Egg::PNTTB_Vertex::type: 
			{
				g->AddInputElement(geometry, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);
				g->AddInputElement(geometry, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12);
				g->AddInputElement(geometry, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24);
				g->AddInputElement(geometry, "TANGENT", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				g->AddInputElement(geometry, "BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 44);
			}
			break;
			case Egg::PNTWBTB_Vertex::type:
			{

				g->AddInputElement(geometry, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0);
				g->AddInputElement(geometry, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12);
				g->AddInputElement(geometry, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24);
				g->AddInputElement(geometry, "WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT, 32);
				g->AddInputElement(geometry, "BONEIDS", DXGI_FORMAT_R32G32B32A32_SINT, 44);
				g->AddInputElement(geometry, "TANGENT", DXGI_FORMAT_R32G32B32_FLOAT, 60);
				g->AddInputElement(geometry, "BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 72);
			}
			break;
			default:
				ASSERT(false, "Vertex type not found");
				break;
		}

		for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
			BYTE * const baseptr = reinterpret_cast<BYTE *>(mesh->vertices);
			g->AddVertexBufferLOD(geometry, (baseptr + mesh->vertexSize * mesh->lodLevels[i].vertexOffset), mesh->lodLevels[i].verticesLength, mesh->vertexSize);
		
			if(mesh->indices != nullptr) {
				BYTE * const ibp = reinterpret_cast<BYTE *>(mesh->indices);
				g->AddIndexBufferLOD(geometry, (ibp + 4U * mesh->lodLevels[i].indexOffset), mesh->lodLevels[i].indicesLength, DXGI_FORMAT_R32_UINT);
			}
		}

		{
			auto preproc = GetPreprocDefs(mesh, mat, model);

			Egg::HINCOMPLETESHADER ivs = g->CreateVertexShader();
			g->SetShaderSource(ivs, L"EggShaderLib.hlsli");
			g->SetShaderEntry(ivs, "Vertex_Main");
			g->SetShaderMacros(ivs, preproc);
			Egg::HSHADER vs = g->CompileShader(ivs);

			Egg::HINCOMPLETESHADER ips = g->CreatePixelShader();
			g->SetShaderSource(ips, L"EggShaderLib.hlsli");
			g->SetShaderEntry(ips, "Pixel_Main");
			g->SetShaderMacros(ips, preproc);
			Egg::HSHADER ps = g->CompileShader(ips);

			Egg::HPSO pso = g->CreatePipelineState();
			g->SetVertexShader(pso, vs);
			g->SetPixelShader(pso, ps);

			Egg::HMATERIAL gmat = g->CreateMaterial(pso, geometry);
			g->SetGeometry(item, geometry);
			g->SetMaterial(item, gmat);
		}

		Egg::HCBUFFER matcb = g->AllocateCbuffer(sizeof(MaterialCb));
		MaterialCb * matcbV = reinterpret_cast<MaterialCb *>(g->GetCbufferPointer(matcb));

		matcbV->diffuseColor = DirectX::XMFLOAT4A{ mat->diffuseColor.x, mat->diffuseColor.y,mat->diffuseColor.z, 1.0f };
		matcbV->fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
		matcbV->shininess = mat->shininess;

		g->AddCbuffer(item, matcb, g->GetCbufferSlot(item, "MaterialCb"));
		
		modelComponent->AddShadedMesh(item, matcbV);
	}

}
