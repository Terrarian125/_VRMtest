#include "VRMModel.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>


//==================================================
// コンストラクタ
//==================================================
VRMModel::VRMModel()

	: m_loaded(false)
	, m_version(Version::Unknown)
	, m_filePath()
	, m_lastError()
	, m_nodeCount(0)
	, m_meshCount(0)
	, m_document()
	, m_vrm0()
	, m_vrm1()
	, m_renderPrimitives()

{
}


//==================================================
// デストラクタ
//==================================================
VRMModel::~VRMModel()
{
	Release();
}


//==================================================
// VRM読み込み
//==================================================
bool VRMModel::Load(
	const std::string& filePath)
{
	Release();

	m_lastError.clear();
	m_filePath = filePath;


	try
	{
		//==================================================
		// VRMはGLB形式なのでfx-gltfで読み込む
		//==================================================
		m_document =
			fx::gltf::LoadFromBinary(filePath);


		//==================================================
		// glTF基本情報
		//==================================================
		m_nodeCount =
			m_document.nodes.size();

		m_meshCount =
			m_document.meshes.size();


		//==================================================
		// extensions取得
		//==================================================
		const nlohmann::json& extensions =
			m_document.extensionsAndExtras["extensions"];


		//==================================================
		// VRM 1.0
		//==================================================
		if (extensions.contains("VRMC_vrm"))
		{
			if (!LoadVRM1(extensions))
			{
				return false;
			}

			m_version =
				Version::VRM_1_0;
		}


		//==================================================
		// VRM 0.x
		//==================================================
		else if (extensions.contains("VRM"))
		{
			if (!LoadVRM0(extensions))
			{
				return false;
			}

			m_version =
				Version::VRM_0_X;
		}


		//==================================================
		// VRM拡張がない
		//==================================================
		else
		{
			SetError(
				"VRM extension was not found.");

			return false;
		}


		//==================================================
		// 描画データ生成
		//==================================================
		if (!BuildRenderData())
		{
			return false;
		}


		//==================================================
		// 読み込み完了
		//==================================================
		m_loaded = true;

		return true;
	}
	catch (const std::exception& e)
	{
		SetError(e.what());

		return false;
	}
	catch (...)
	{
		SetError(
			"Unknown exception occurred while loading VRM.");

		return false;
	}
}


//==================================================
// VRM 0.x読み込み
//==================================================
bool VRMModel::LoadVRM0(
	const nlohmann::json& extensions)
{
	try
	{
		VRMC_VRM_0_0::from_json(
			extensions["VRM"],
			m_vrm0);

		return true;
	}
	catch (const std::exception& e)
	{
		SetError(e.what());

		return false;
	}
	catch (...)
	{
		SetError(
			"Failed to deserialize VRM 0.x data.");

		return false;
	}
}


//==================================================
// VRM 1.0読み込み
//==================================================
bool VRMModel::LoadVRM1(
	const nlohmann::json& extensions)
{
	try
	{
		VRMC_VRM_1_0::from_json(
			extensions["VRMC_vrm"],
			m_vrm1);

		return true;
	}
	catch (const std::exception& e)
	{
		SetError(e.what());

		return false;
	}
	catch (...)
	{
		SetError(
			"Failed to deserialize VRM 1.0 data.");

		return false;
	}
}


//==================================================
// 描画データ生成
//==================================================
bool VRMModel::BuildRenderData()
{
	m_renderPrimitives.clear();


	//==================================================
	// ノードを調べる
	//==================================================
	for (size_t nodeIndex = 0;
		nodeIndex < m_document.nodes.size();
		++nodeIndex)
	{
		const fx::gltf::Node& node =
			m_document.nodes[nodeIndex];


		//==================================================
		// メッシュを持っていないノードは無視
		//==================================================
		if (node.mesh < 0)
		{
			continue;
		}


		if (static_cast<size_t>(node.mesh) >=
			m_document.meshes.size())
		{
			SetError(
				"Node references an invalid mesh.");

			return false;
		}


		const fx::gltf::Mesh& mesh =
			m_document.meshes[node.mesh];


		//==================================================
		// メッシュ内のPrimitiveを処理
		//==================================================
		for (const fx::gltf::Primitive& primitive :
			mesh.primitives)
		{
			//==================================================
			// 現段階では三角形のみ対応
			//==================================================
			if (primitive.mode !=
				fx::gltf::Primitive::Mode::Triangles)
			{
				continue;
			}


			RenderPrimitive renderPrimitive;


			if (!BuildPrimitive(
				primitive,
				static_cast<int>(nodeIndex),
				renderPrimitive))
			{
				return false;
			}


			if (renderPrimitive.vertices.empty() ||
				renderPrimitive.indices.empty())
			{
				continue;
			}


			m_renderPrimitives.push_back(
				std::move(renderPrimitive));
		}
	}


	//==================================================
	// メッシュが一つもなかった
	//==================================================
	if (m_renderPrimitives.empty())
	{
		SetError(
			"No renderable triangle primitives were found.");

		return false;
	}


	return true;
}


//==================================================
// プリミティブ描画データ生成
//==================================================
bool VRMModel::BuildPrimitive(
	const fx::gltf::Primitive& primitive,
	int nodeIndex,
	RenderPrimitive& output)
{
	output.vertices.clear();
	output.indices.clear();

	output.materialIndex =
		primitive.material;

	output.nodeIndex =
		nodeIndex;


	//==================================================
	// POSITION取得
	//==================================================
	const auto positionIt =
		primitive.attributes.find("POSITION");


	if (positionIt ==
		primitive.attributes.end())
	{
		SetError(
			"Primitive does not contain POSITION.");

		return false;
	}


	std::vector<DxLib::VECTOR> positions;


	if (!ReadVec3Accessor(
		static_cast<int>(positionIt->second),
		positions))
	{
		return false;
	}


	//==================================================
	// NORMAL取得
	//==================================================
	std::vector<DxLib::VECTOR> normals;


	const auto normalIt =
		primitive.attributes.find("NORMAL");


	if (normalIt !=
		primitive.attributes.end())
	{
		if (!ReadVec3Accessor(
			static_cast<int>(normalIt->second),
			normals))
		{
			return false;
		}
	}


	//==================================================
	// TEXCOORD_0取得
	//==================================================
	std::vector<std::pair<float, float>> texcoords;


	const auto texcoordIt =
		primitive.attributes.find("TEXCOORD_0");


	if (texcoordIt !=
		primitive.attributes.end())
	{
		if (!ReadVec2Accessor(
			static_cast<int>(texcoordIt->second),
			texcoords))
		{
			return false;
		}
	}


	//==================================================
	// インデックス取得
	//==================================================
	if (primitive.indices < 0)
	{
		SetError(
			"Primitive does not contain indices.");

		return false;
	}


	if (!ReadIndexAccessor(
		primitive.indices,
		output.indices))
	{
		return false;
	}


	//==================================================
	// DxLibの16bit Index制限
	//==================================================
	if (positions.size() > 65535)
	{
		SetError(
			"A primitive contains more than 65535 vertices.");

		return false;
	}


	//==================================================
	// インデックス範囲確認
	//==================================================
	for (unsigned short index :
	output.indices)
	{
		if (index >= positions.size())
		{
			SetError(
				"Primitive index is out of vertex range.");

			return false;
		}
	}


	//==================================================
	// 頂点生成
	//==================================================
	output.vertices.resize(
		positions.size());


	for (size_t i = 0;
		i < positions.size();
		++i)
	{
		DxLib::VERTEX3D vertex{};


		//==================================================
		// 座標
		//==================================================
		vertex.pos =
			positions[i];


		//==================================================
		// 法線
		//==================================================
		if (i < normals.size())
		{
			vertex.norm =
				normals[i];
		}
		else
		{
			vertex.norm =
				VGet(
					0.0f,
					1.0f,
					0.0f);
		}


		//==================================================
		// 頂点カラー
		//==================================================
		vertex.dif =
			GetColorU8(
				255,
				255,
				255,
				255);


		vertex.spc =
			GetColorU8(
				0,
				0,
				0,
				0);


		//==================================================
		// UV
		//==================================================
		if (i < texcoords.size())
		{
			vertex.u =
				texcoords[i].first;

			vertex.v =
				texcoords[i].second;
		}
		else
		{
			vertex.u =
				0.0f;

			vertex.v =
				0.0f;
		}


		vertex.su =
			0.0f;

		vertex.sv =
			0.0f;


		output.vertices[i] =
			vertex;
	}


	return true;
}


//==================================================
// アクセサからVec3を取得
//==================================================
bool VRMModel::ReadVec3Accessor(
	int accessorIndex,
	std::vector<DxLib::VECTOR>& output) const
{
	output.clear();


	//==================================================
	// アクセサ番号確認
	//==================================================
	if (accessorIndex < 0 ||
		static_cast<size_t>(accessorIndex) >=
		m_document.accessors.size())
	{
		return false;
	}


	const fx::gltf::Accessor& accessor =
		m_document.accessors[accessorIndex];


	//==================================================
	// VEC3確認
	//==================================================
	if (accessor.type !=
		fx::gltf::Accessor::Type::Vec3)
	{
		return false;
	}


	//==================================================
	// float確認
	//==================================================
	if (accessor.componentType !=
		fx::gltf::Accessor::ComponentType::Float)
	{
		return false;
	}


	//==================================================
	// BufferView確認
	//==================================================
	if (accessor.bufferView < 0 ||
		static_cast<size_t>(accessor.bufferView) >=
		m_document.bufferViews.size())
	{
		return false;
	}


	const fx::gltf::BufferView& bufferView =
		m_document.bufferViews[
			accessor.bufferView];


	//==================================================
	// Buffer確認
	//==================================================
	if (bufferView.buffer < 0 ||
		static_cast<size_t>(bufferView.buffer) >=
		m_document.buffers.size())
	{
		return false;
	}


	const fx::gltf::Buffer& buffer =
		m_document.buffers[
			bufferView.buffer];


	//==================================================
	// 1頂点あたりのサイズ
	//==================================================
	const size_t elementSize =
		sizeof(float) * 3;


	//==================================================
	// Stride
	//==================================================
	const size_t stride =
		bufferView.byteStride != 0
		? bufferView.byteStride
		: elementSize;


	//==================================================
	// データ開始位置
	//==================================================
	const size_t startOffset =
		static_cast<size_t>(
			bufferView.byteOffset) +
		static_cast<size_t>(
			accessor.byteOffset);


	//==================================================
	// データサイズ確認
	//==================================================
	const size_t requiredSize =
		accessor.count == 0
		? 0
		: startOffset +
		stride * (accessor.count - 1) +
		elementSize;


	if (requiredSize >
		buffer.data.size())
	{
		return false;
	}


	//==================================================
	// データ読み込み
	//==================================================
	output.resize(
		accessor.count);


	for (size_t i = 0;
		i < accessor.count;
		++i)
	{
		const uint8_t* src =
			buffer.data.data() +
			startOffset +
			stride * i;


		float values[3];


		std::memcpy(
			&values[0],
			src + sizeof(float) * 0,
			sizeof(float));


		std::memcpy(
			&values[1],
			src + sizeof(float) * 1,
			sizeof(float));


		std::memcpy(
			&values[2],
			src + sizeof(float) * 2,
			sizeof(float));


		output[i] =
			VGet(
				values[0],
				values[1],
				values[2]);
	}


	return true;
}


//==================================================
// アクセサからVec2を取得
//==================================================
bool VRMModel::ReadVec2Accessor(
	int accessorIndex,
	std::vector<std::pair<float, float>>& output) const
{
	output.clear();


	//==================================================
	// アクセサ番号確認
	//==================================================
	if (accessorIndex < 0 ||
		static_cast<size_t>(accessorIndex) >=
		m_document.accessors.size())
	{
		return false;
	}


	const fx::gltf::Accessor& accessor =
		m_document.accessors[accessorIndex];


	//==================================================
	// VEC2確認
	//==================================================
	if (accessor.type !=
		fx::gltf::Accessor::Type::Vec2)
	{
		return false;
	}


	//==================================================
	// float確認
	//==================================================
	if (accessor.componentType !=
		fx::gltf::Accessor::ComponentType::Float)
	{
		return false;
	}


	//==================================================
	// BufferView確認
	//==================================================
	if (accessor.bufferView < 0 ||
		static_cast<size_t>(accessor.bufferView) >=
		m_document.bufferViews.size())
	{
		return false;
	}


	const fx::gltf::BufferView& bufferView =
		m_document.bufferViews[
			accessor.bufferView];


	//==================================================
	// Buffer確認
	//==================================================
	if (bufferView.buffer < 0 ||
		static_cast<size_t>(bufferView.buffer) >=
		m_document.buffers.size())
	{
		return false;
	}


	const fx::gltf::Buffer& buffer =
		m_document.buffers[
			bufferView.buffer];


	//==================================================
	// 1頂点あたりのサイズ
	//==================================================
	const size_t elementSize =
		sizeof(float) * 2;


	//==================================================
	// Stride
	//==================================================
	const size_t stride =
		bufferView.byteStride != 0
		? bufferView.byteStride
		: elementSize;


	//==================================================
	// データ開始位置
	//==================================================
	const size_t startOffset =
		static_cast<size_t>(
			bufferView.byteOffset) +
		static_cast<size_t>(
			accessor.byteOffset);


	//==================================================
	// データサイズ確認
	//==================================================
	const size_t requiredSize =
		accessor.count == 0
		? 0
		: startOffset +
		stride * (accessor.count - 1) +
		elementSize;


	if (requiredSize >
		buffer.data.size())
	{
		return false;
	}


	//==================================================
	// データ読み込み
	//==================================================
	output.resize(
		accessor.count);


	for (size_t i = 0;
		i < accessor.count;
		++i)
	{
		const uint8_t* src =
			buffer.data.data() +
			startOffset +
			stride * i;


		float u;
		float v;


		std::memcpy(
			&u,
			src,
			sizeof(float));


		std::memcpy(
			&v,
			src + sizeof(float),
			sizeof(float));


		output[i] =
			std::make_pair(
				u,
				v);
	}


	return true;
}


//==================================================
// インデックスアクセサ取得
//==================================================
bool VRMModel::ReadIndexAccessor(
	int accessorIndex,
	std::vector<unsigned short>& output) const
{
	output.clear();


	//==================================================
	// アクセサ番号確認
	//==================================================
	if (accessorIndex < 0 ||
		static_cast<size_t>(accessorIndex) >=
		m_document.accessors.size())
	{
		return false;
	}


	const fx::gltf::Accessor& accessor =
		m_document.accessors[accessorIndex];


	//==================================================
	// Scalar確認
	//==================================================
	if (accessor.type !=
		fx::gltf::Accessor::Type::Scalar)
	{
		return false;
	}


	//==================================================
	// BufferView確認
	//==================================================
	if (accessor.bufferView < 0 ||
		static_cast<size_t>(accessor.bufferView) >=
		m_document.bufferViews.size())
	{
		return false;
	}


	const fx::gltf::BufferView& bufferView =
		m_document.bufferViews[
			accessor.bufferView];


	//==================================================
	// Buffer確認
	//==================================================
	if (bufferView.buffer < 0 ||
		static_cast<size_t>(bufferView.buffer) >=
		m_document.buffers.size())
	{
		return false;
	}


	const fx::gltf::Buffer& buffer =
		m_document.buffers[
			bufferView.buffer];


	//==================================================
	// コンポーネントサイズ
	//==================================================
	size_t componentSize = 0;


	switch (accessor.componentType)
	{
	case fx::gltf::Accessor::ComponentType::UnsignedByte:

		componentSize =
			sizeof(uint8_t);

		break;


	case fx::gltf::Accessor::ComponentType::UnsignedShort:

		componentSize =
			sizeof(uint16_t);

		break;


	case fx::gltf::Accessor::ComponentType::UnsignedInt:

		componentSize =
			sizeof(uint32_t);

		break;


	default:

		return false;
	}


	//==================================================
	// Stride
	//==================================================
	const size_t stride =
		bufferView.byteStride != 0
		? bufferView.byteStride
		: componentSize;


	//==================================================
	// データ開始位置
	//==================================================
	const size_t startOffset =
		static_cast<size_t>(
			bufferView.byteOffset) +
		static_cast<size_t>(
			accessor.byteOffset);


	//==================================================
	// データサイズ確認
	//==================================================
	const size_t requiredSize =
		accessor.count == 0
		? 0
		: startOffset +
		stride * (accessor.count - 1) +
		componentSize;


	if (requiredSize >
		buffer.data.size())
	{
		return false;
	}


	//==================================================
	// データ読み込み
	//==================================================
	output.resize(
		accessor.count);


	for (size_t i = 0;
		i < accessor.count;
		++i)
	{
		const uint8_t* src =
			buffer.data.data() +
			startOffset +
			stride * i;


		uint32_t value = 0;


		switch (accessor.componentType)
		{
		case fx::gltf::Accessor::ComponentType::UnsignedByte:

			value =
				*src;

			break;


		case fx::gltf::Accessor::ComponentType::UnsignedShort:
		{
			uint16_t temp;


			std::memcpy(
				&temp,
				src,
				sizeof(uint16_t));


			value =
				temp;

			break;
		}


		case fx::gltf::Accessor::ComponentType::UnsignedInt:
		{
			uint32_t temp;


			std::memcpy(
				&temp,
				src,
				sizeof(uint32_t));


			value =
				temp;

			break;
		}


		default:

			return false;
		}


		//==================================================
		// DxLibは16bit Indexを使用
		//==================================================
		if (value > 65535)
		{
			return false;
		}


		output[i] =
			static_cast<unsigned short>(
				value);
	}


	return true;
}


//==================================================
// 解放
//==================================================
void VRMModel::Release()
{
	m_loaded = false;

	m_version =
		Version::Unknown;

	m_filePath.clear();

	m_lastError.clear();

	m_nodeCount = 0;

	m_meshCount = 0;

	m_renderPrimitives.clear();

	m_document =
		fx::gltf::Document();

	m_vrm0 =
		VRMC_VRM_0_0::Vrm();

	m_vrm1 =
		VRMC_VRM_1_0::Vrm();
}


//==================================================
// 読み込み済みか
//==================================================
bool VRMModel::IsLoaded() const
{
	return m_loaded;
}


//==================================================
// VRMバージョン取得
//==================================================
VRMModel::Version
VRMModel::GetVersion() const
{
	return m_version;
}


//==================================================
// VRM 1.0取得
//==================================================
const VRMC_VRM_1_0::Vrm*
VRMModel::GetVRM1() const
{
	if (m_version != Version::VRM_1_0)
	{
		return nullptr;
	}

	return &m_vrm1;
}


//==================================================
// VRM 0.x取得
//==================================================
const VRMC_VRM_0_0::Vrm*
VRMModel::GetVRM0() const
{
	if (m_version != Version::VRM_0_X)
	{
		return nullptr;
	}

	return &m_vrm0;
}


//==================================================
// モデル名取得
//==================================================
std::string
VRMModel::GetModelName() const
{
	if (m_version == Version::VRM_1_0)
	{
		return m_vrm1.meta.name;
	}


	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.meta.title;
	}


	return "";
}


//==================================================
// 作者名取得
//==================================================
std::string
VRMModel::GetAuthorName() const
{
	if (m_version == Version::VRM_1_0)
	{
		if (!m_vrm1.meta.authors.empty())
		{
			return m_vrm1.meta.authors[0];
		}

		return "";
	}


	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.meta.author;
	}


	return "";
}


//==================================================
// Humanoidボーン数取得
//==================================================
size_t
VRMModel::GetHumanoidBoneCount() const
{
	if (m_version == Version::VRM_1_0)
	{
		size_t count = 0;

		const auto& bones =
			m_vrm1.humanoid.humanBones;


#define COUNT_BONE(boneName) \
		if (bones.boneName.node != 0) \
		{ \
			++count; \
		}


		COUNT_BONE(hips);
		COUNT_BONE(spine);
		COUNT_BONE(chest);
		COUNT_BONE(upperChest);
		COUNT_BONE(neck);
		COUNT_BONE(head);

		COUNT_BONE(leftEye);
		COUNT_BONE(rightEye);
		COUNT_BONE(jaw);

		COUNT_BONE(leftUpperLeg);
		COUNT_BONE(leftLowerLeg);
		COUNT_BONE(leftFoot);
		COUNT_BONE(leftToes);

		COUNT_BONE(rightUpperLeg);
		COUNT_BONE(rightLowerLeg);
		COUNT_BONE(rightFoot);
		COUNT_BONE(rightToes);

		COUNT_BONE(leftShoulder);
		COUNT_BONE(leftUpperArm);
		COUNT_BONE(leftLowerArm);
		COUNT_BONE(leftHand);

		COUNT_BONE(rightShoulder);
		COUNT_BONE(rightUpperArm);
		COUNT_BONE(rightLowerArm);
		COUNT_BONE(rightHand);


#undef COUNT_BONE


		return count;
	}


	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.humanoid.humanBones.size();
	}


	return 0;
}


//==================================================
// glTFノード数取得
//==================================================
size_t
VRMModel::GetNodeCount() const
{
	return m_nodeCount;
}


//==================================================
// glTFメッシュ数取得
//==================================================
size_t
VRMModel::GetMeshCount() const
{
	return m_meshCount;
}


//==================================================
// 描画プリミティブ数取得
//==================================================
size_t
VRMModel::GetRenderPrimitiveCount() const
{
	return m_renderPrimitives.size();
}


//==================================================
// エラー取得
//==================================================
const std::string&
VRMModel::GetLastError() const
{
	return m_lastError;
}


//==================================================
// エラー設定
//==================================================
void VRMModel::SetError(
	const std::string& message)
{
	m_lastError =
		message;
}


//==================================================
// 描画
//==================================================
void VRMModel::Draw(
	float scale,
	DxLib::VECTOR position,
	float rotationY)
{
	if (!m_loaded)
	{
		return;
	}


	//==================================================
	// 現段階ではスキニング前の
	// 静的メッシュとして描画する
	//==================================================
	for (const RenderPrimitive& primitive :
		m_renderPrimitives)
	{
		if (primitive.vertices.empty() ||
			primitive.indices.empty())
		{
			continue;
		}


		std::vector<DxLib::VERTEX3D> vertices =
			primitive.vertices;


		//==================================================
		// ノード情報
		//==================================================
		if (primitive.nodeIndex >= 0 &&
			static_cast<size_t>(primitive.nodeIndex) <
			m_document.nodes.size())
		{
			const fx::gltf::Node& node =
				m_document.nodes[
					primitive.nodeIndex];


			//==================================================
			// ノードのScaleを反映
			//==================================================
			for (DxLib::VERTEX3D& vertex :
				vertices)
			{
				vertex.pos.x *=
					node.scale[0];

				vertex.pos.y *=
					node.scale[1];

				vertex.pos.z *=
					node.scale[2];


				//==================================================
				// ノードのTranslationを反映
				//==================================================
				vertex.pos.x +=
					node.translation[0];

				vertex.pos.y +=
					node.translation[1];

				vertex.pos.z +=
					node.translation[2];
			}
		}


		//==================================================
		// ゲーム側のScaleを反映
		//==================================================
		for (DxLib::VERTEX3D& vertex :
			vertices)
		{
			vertex.pos =
				VScale(
					vertex.pos,
					scale);
		}


		//==================================================
		// ゲーム側のY回転
		//==================================================
		const float cosY =
			cosf(rotationY);

		const float sinY =
			sinf(rotationY);


		for (DxLib::VERTEX3D& vertex :
			vertices)
		{
			const float x =
				vertex.pos.x;

			const float z =
				vertex.pos.z;


			vertex.pos.x =
				x * cosY -
				z * sinY;

			vertex.pos.z =
				x * sinY +
				z * cosY;


			//==================================================
			// 位置
			//==================================================
			vertex.pos =
				VAdd(
					vertex.pos,
					position);
		}


		//==================================================
		// ひとまず両面描画
		//==================================================
		SetUseBackCulling(
			FALSE);


		//==================================================
		// テクスチャなしで描画
		//==================================================
		DrawPolygonIndexed3D(
			vertices.data(),
			static_cast<int>(
				vertices.size()),
			const_cast<unsigned short*>(
				primitive.indices.data()),
			static_cast<int>(
				primitive.indices.size() / 3),
			DX_NONE_GRAPH,
			FALSE);


		//==================================================
		// カリング設定を戻す
		//==================================================
		SetUseBackCulling(
			TRUE);
	}
}