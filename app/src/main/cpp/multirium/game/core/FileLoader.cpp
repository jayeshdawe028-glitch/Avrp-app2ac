//
// Created by viski on 02.06.2025.
//

#include "FileLoader.h"
#include "main.h"
#include <fstream>
#include "FileMgr.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/Enums/eItemDefinitionFlags.h"
#include "vendor/armhook/patch.h"
#include "game/modelinfo/AtomicModelInfo.h"
#include "Streaming.h"
#include "util/util.h"

// Load line into static buffer (`ms_line`)
char* CFileLoader::LoadLine(FILE* file) {
   // if(!fgets(ms_line, sizeof(ms_line), file))
     //   return nullptr;

  //  return ms_line;

    if (!CFileMgr::ReadLine(file, ms_line, sizeof(ms_line)))
        return nullptr;

    // Sanitize it (otherwise random crashes appear)
    for (char* it = ms_line; *it; it++) {
        // Have to cast to uint8, because signed ASCII is retarded
        if ((uint8)*it < (uint8)' ' || *it == ',')
            *it = ' ';
    }

    return FindFirstNonNullOrWS(ms_line);
  //  return ( ( char*(*)(FILE*) )(g_libGTASA + 0x00394A30 + 1) )(file);
}

bool CFileMgr::ReadLine(FILESTREAM file, char *str, int32 num)
{
    return fgets(str, num, file) != nullptr;
}

char* CFileLoader::LoadLine(char*& bufferIt, int32& buffSize) {
    return ( ( char*(*)(char*, int32) )(g_libGTASA + (VER_x32 ? 0x003EEFD8 + 1 : 0x4D0108)) )(bufferIt, buffSize);
}

char* CFileLoader::FindFirstNonNullOrWS(char* it) {
    for (; *it && (uint8)*it <= (uint8)' '; it++);
    return it;
}

int32 CFileLoader::LoadObject(const char* line) {
    int32  modelId{ -1};
    char   modelName[24];
    char   texName[24];
    float  fDrawDist;
    uint32 nFlags;

    auto iNumRead = sscanf(line, "%d %s %s %f %d", &modelId, modelName, texName, &fDrawDist, &nFlags);
    if (iNumRead != 5 || fDrawDist < 4.0f)
    {
        int32 objType;
        float fDrawDist2_unused, fDrawDist3_unused;
        iNumRead = sscanf(line, "%d %s %s %d", &modelId, modelName, texName, &objType);
        if (iNumRead != 4)
            return -1;

        switch (objType)
        {
            case 1:
                VERIFY(sscanf(line, "%d %s %s %d %f %d", &modelId, (modelName), (texName), &objType, &fDrawDist, &nFlags) >= 5);
                break;
            case 2:
                VERIFY(sscanf(line, "%d %s %s %d %f %f %d", &modelId, (modelName), (texName), &objType, &fDrawDist, &fDrawDist2_unused, &nFlags) == 7);
                break;
            case 3:
                VERIFY(sscanf(line, "%d %s %s %d %f %f %f %d", &modelId, (modelName), (texName), &objType, &fDrawDist, &fDrawDist2_unused, &fDrawDist3_unused, &nFlags) == 8);
                break;
        }
    }

    sItemDefinitionFlags flags(nFlags);
    const auto mi = flags.bIsDamageable ? reinterpret_cast<CAtomicModelInfo *>(CModelInfo::AddDamageAtomicModel(modelId)) : CModelInfo::AddAtomicModel(modelId);
    mi->m_lodDistance = fDrawDist;

    mi->SetModelName(modelName);

    auto db = CStreaming::GetModelCDName(modelId);
    mi->SetTexDictionary(texName, db);

    SetAtomicModelInfoFlags(mi, nFlags);

    return modelId;
}

CEntity* CFileLoader::LoadObjectInstance1(const char* line) {
    char modelName[24];
    CFileObjectInstance instance;
    VERIFY(sscanf(
            line,
            "%d %s %d %f %f %f %f %f %f %f %d",
            &instance.m_nModelId,
            &modelName,
            &instance.m_nInstanceType,
            &instance.m_vecPosition.x,
            &instance.m_vecPosition.y,
            &instance.m_vecPosition.z,
            &instance.m_qRotation.x,
            &instance.m_qRotation.y,
            &instance.m_qRotation.z,
            &instance.m_qRotation.w,
            &instance.m_nLodInstanceIndex
    ) == 11);

    return LoadObjectInstance(&instance, modelName);
}

bool isLoadedLevel = false;

int CFileLoader::LoadPedModels() {
    Log("CFileLoader::LoadPedModels()");

    std::string jsonPath = std::string(g_pszStorage) + "data/ped_models.json";

    std::ifstream jsonFile(jsonPath);
    if (!jsonFile.is_open())
        return 0;

    nlohmann::json pedModelsJson;
    try {
        jsonFile >> pedModelsJson;
    } catch (...) {
        jsonFile.close();
        return 0;
    }

    for (auto& pedModelData : pedModelsJson) {
        int modelId = -1;
        std::string modelName;
        std::string txdName;
        std::string animGroup;

        if (pedModelData.contains("id")) {
            modelId = pedModelData["id"].get<int>();
        }

        if (pedModelData.contains("modelName")) {
            modelName = pedModelData["modelName"].get<std::string>();
        }

        if (pedModelData.contains("txdName")) {
            txdName = pedModelData["txdName"].get<std::string>();
        }

        if (pedModelData.contains("animGroup")) {
            animGroup = pedModelData["animGroup"].get<std::string>();
        }

        CPedModelInfo* pedModelInfo = CModelInfo::AddPedModel(modelId);
        if (!pedModelInfo) {
            continue;
        }

        pedModelInfo->m_nStatType = CKeyGen::GetUppercaseKey(modelName.c_str());

        CStreaming::GetModelCDName(modelId);

        //CBaseModelInfo::SetTexDictionary(
          //      txdName.c_str(),
         //       "skins"
       // );

        if (pedModelData.contains("animFile") && !pedModelData["animFile"].empty()) {
            std::string animFile = pedModelData["animFile"].get<std::string>();

          //  reinterpret_cast<CBaseModelInfo*>(pedModelInfo)->SetAnimFile(animFile.c_str());
        }

       // CBaseModelInfo::SetColModel(
        //        &CTempColModels::ms_colModelPeds,
       //         false
       // );

        strncpy((char*)&pedModelInfo->m_nCarsCanDriveMask, modelName.c_str(), 20);
        ((char*)&pedModelInfo->m_nCarsCanDriveMask)[20] = '\0';

        *reinterpret_cast<uint32_t*>(pedModelInfo[2].m_nCarsCanDriveMask) = 0;
        *reinterpret_cast<uint32_t*>(pedModelInfo[2].m_pHitColModel) = 0;
        *reinterpret_cast<uint32_t*>(pedModelInfo[2].m_pHitColModel) = 0;

        //int animGroupId = CAnimManager::GetAnimationGroupId(animGroup.c_str());
       // *reinterpret_cast<uint32_t*>(pedModelInfo[2].m_nStatType) = animGroupId;

        *reinterpret_cast<uint16_t*>(pedModelInfo[2].m_LastVoice) = 0;
        *reinterpret_cast<uint8_t*>(pedModelInfo[2].m_NextVoice) = 0;
        *reinterpret_cast<uint64_t*>(pedModelInfo[3].m_nAnimType) = 0;
    }

    jsonFile.close();

    return 1;
}

void(*CFileLoader__LoadLevel)(const char *pDatFilename);
void CFileLoader__LoadLevel_hook(const char *pDatFilename)
{
    Log(OBFUSCATE("LoadLevel %s"), pDatFilename);
    if ( !isLoadedLevel )
    {
        isLoadedLevel = true;
       // CFileLoader::LoadPedModels();
    }
    return CFileLoader__LoadLevel(pDatFilename);
}

void CFileLoader::InjectHooks() {
    CHook::InlineHook("_ZN11CFileLoader9LoadLevelEPKc", CFileLoader__LoadLevel_hook, &CFileLoader__LoadLevel);
   // CHook::Redirect("_ZN11CFileLoader10LoadObjectEPKc", &CFileLoader::LoadObject);
   // CHook::Redirect("_ZN11CFileLoader18LoadObjectInstanceEPKc", &CFileLoader::LoadObjectInstance1);
}

CEntity *CFileLoader::LoadObjectInstance(CFileObjectInstance *objInstance, const char *modelName) {
    return CHook::CallFunction<CEntity*>(g_libGTASA + (VER_x32 ? 0x003F059C + 1 : 0x4D20FC), objInstance, modelName);
}
