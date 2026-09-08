#include "VRMModel.h"

#include <fx/gltf.h>

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
	, m_vrm0()
	, m_vrm1()
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
	//--------------------------------------------------
	// 前回のデータを破棄
	//--------------------------------------------------
	Release();

	m_lastError.clear();
	m_filePath = filePath;


	try
	{
		//--------------------------------------------------
		// VRMは実体としてGLBなので、
		// fx-gltfでバイナリGLTFとして読み込む。
		//--------------------------------------------------
		const fx::gltf::Document document =
			fx::gltf::LoadFromBinary(filePath);


		//--------------------------------------------------
		// 基本的なGLTF情報を保存
		//--------------------------------------------------
		m_nodeCount =
			document.nodes.size();

		m_meshCount =
			document.meshes.size();


		//--------------------------------------------------
		// extensions / extras
		//
		// VRM.h公式サンプルも
		//
		// doc.extensionsAndExtras["extensions"]
		//
		// からVRM拡張を取得している。
		//--------------------------------------------------
		const nlohmann::json& extensions =
			document.extensionsAndExtras["extensions"];


		//--------------------------------------------------
		// VRM 1.0
		//--------------------------------------------------
		if (extensions.contains("VRMC_vrm"))
		{
			if (!LoadVRM1(extensions))
			{
				return false;
			}

			m_version =
				Version::VRM_1_0;

			m_loaded = true;

			return true;
		}


		//--------------------------------------------------
		// VRM 0.x
		//--------------------------------------------------
		if (extensions.contains("VRM"))
		{
			if (!LoadVRM0(extensions))
			{
				return false;
			}

			m_version =
				Version::VRM_0_X;

			m_loaded = true;

			return true;
		}


		//--------------------------------------------------
		// VRM拡張が存在しない
		//--------------------------------------------------
		SetError(
			"VRM extension was not found.");

		return false;
	}
	catch (const std::exception& e)
	{
		//--------------------------------------------------
		// fx-gltf / VRM.h / JSON等で発生した例外
		//--------------------------------------------------
		SetError(e.what());

		return false;
	}
	catch (...)
	{
		//--------------------------------------------------
		// 不明な例外
		//--------------------------------------------------
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
		//--------------------------------------------------
		// VRM 0.x
		//
		// 拡張名：
		// "VRM"
		//--------------------------------------------------
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
		//--------------------------------------------------
		// VRM 1.0
		//
		// 拡張名：
		// "VRMC_vrm"
		//--------------------------------------------------
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
// 解放
//==================================================
void VRMModel::Release()
{
	m_loaded = false;

	m_version =
		Version::Unknown;

	m_filePath.clear();

	m_nodeCount = 0;

	m_meshCount = 0;

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
// VRM 1.0情報取得
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
// VRM 0.x情報取得
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
	//--------------------------------------------------
	// VRM 1.0
	//--------------------------------------------------
	if (m_version == Version::VRM_1_0)
	{
		return m_vrm1.meta.name;
	}


	//--------------------------------------------------
	// VRM 0.x
	//--------------------------------------------------
	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.meta.title;
	}


	return "";
}


//==================================================
// 作者取得
//==================================================
std::string
VRMModel::GetAuthorName() const
{
	//--------------------------------------------------
	// VRM 1.0
	//--------------------------------------------------
	if (m_version == Version::VRM_1_0)
	{
		if (!m_vrm1.meta.authors.empty())
		{
			return m_vrm1.meta.authors[0];
		}

		return "";
	}


	//--------------------------------------------------
	// VRM 0.x
	//--------------------------------------------------
	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.meta.author;
	}


	return "";
}


//==================================================
// Humanoidボーン数
//==================================================
size_t
VRMModel::GetHumanoidBoneCount() const
{
	//--------------------------------------------------
	// VRM 1.0
	//--------------------------------------------------
	if (m_version == Version::VRM_1_0)
	{
		//--------------------------------------------------
		// VRM 1.0ではHumanBonesが構造体として
		// 定義されているため、vector.size()では取得できない。
		//
		// 現段階では主要ボーンの存在数を数える。
		//--------------------------------------------------
		size_t count = 0;


#define COUNT_BONE(bone)                                      \
		if (m_vrm1.humanoid.humanBones.bone.node != 0)       \
		{                                                     \
			++count;                                          \
		}


		COUNT_BONE(hips);
		COUNT_BONE(spine);
		COUNT_BONE(chest);
		COUNT_BONE(upperChest);
		COUNT_BONE(neck);
		COUNT_BONE(head);

		COUNT_BONE(leftShoulder);
		COUNT_BONE(leftUpperArm);
		COUNT_BONE(leftLowerArm);
		COUNT_BONE(leftHand);

		COUNT_BONE(rightShoulder);
		COUNT_BONE(rightUpperArm);
		COUNT_BONE(rightLowerArm);
		COUNT_BONE(rightHand);

		COUNT_BONE(leftUpperLeg);
		COUNT_BONE(leftLowerLeg);
		COUNT_BONE(leftFoot);
		COUNT_BONE(leftToes);

		COUNT_BONE(rightUpperLeg);
		COUNT_BONE(rightLowerLeg);
		COUNT_BONE(rightFoot);
		COUNT_BONE(rightToes);

		COUNT_BONE(leftEye);
		COUNT_BONE(rightEye);
		COUNT_BONE(jaw);


#undef COUNT_BONE


		return count;
	}


	//--------------------------------------------------
	// VRM 0.x
	//--------------------------------------------------
	if (m_version == Version::VRM_0_X)
	{
		return m_vrm0.humanoid.humanBones.size();
	}


	return 0;
}


//==================================================
// glTFノード数
//==================================================
size_t
VRMModel::GetNodeCount() const
{
	return m_nodeCount;
}


//==================================================
// glTFメッシュ数
//==================================================
size_t
VRMModel::GetMeshCount() const
{
	return m_meshCount;
}


//==================================================
// エラー内容
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
	m_lastError = message;

	m_loaded = false;

	m_version =
		Version::Unknown;
}